using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;


namespace SliceEngine
{

    public class TestController : Entity, IInitializable
    {
        public float moveSpeed = 2.5f;
        public float rotSpeed = 12f;
        public bool isGrounded = false;
        public bool isJumping = false;
        public bool isDoubleJumping = false;
        public bool isGroundDashing = false;
        public bool isAirDashing = false;
        public bool canInput = true;
        public bool isAttacking = false;
        public GameObject playerModel;
        public GameObject cameraObject;
        public float dashDuration = 0.5f;
        public float dashCooldown = 0.6f;
        public float dashSpeed = 15.0f;

        private float dashCooldownTimer = 0.0f;
        private float dashTimer = 0.0f;
        private Vector3 finalMove;
        private Vector3 input;
        private Vector3 dashInputDir;
        private RigidBody rb;
        private Animator animator;
        private CameraController camera;
        private int groundTriggerCount = 0;
        private string groundName = "Ground";
        private bool jumpRequest = false;
        private bool dashRequest = false;

        #region Entity Overrides
        public void Initialize()
        {

        }
        public override void OnDeath()
        {

        }
        protected override void OnHeal()
        {

        }
        protected override void OnDamaged(GameObject source)
        {

        }
        #endregion

        public override void OnAwake()
        {

        }

        public override void OnCreate()
        {
            animator = playerModel?.GetComponent<Animator>();
            rb = GetComponent<RigidBody>();
            camera = cameraObject.As<CameraController>();
        }

        public override void OnUpdate(float dt)
        {
            if (dashCooldownTimer > 0.0f) dashCooldownTimer -= dt;

            if (canInput)
            {
                HandleInput();
                
                // Capture jump input in OnUpdate to ensure we don't miss a key press
                if (Input.IsKeyPressed(Keys.KEY_SPACEBAR))
                {
                    if (isGrounded && !isAttacking) jumpRequest = true;
                    else if (isJumping && !isDoubleJumping) jumpRequest = true;
                }
            }

            if (isGroundDashing || isAirDashing)
            {
                dashTimer -= dt;
                if (dashTimer <= 0.0f)
                {
                    dashCooldownTimer = dashCooldown;
                    isGroundDashing = false;
                    isAirDashing = false;
                }
            }

            UpdateRotation(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            ApplyDash();
            HandleMovement(dt);
            ApplyJump();
        }

        void HandleInput()
        {
            input = Vector3.Zero;
            // Forward/backward movement
            if (canInput)
            {
                if (Input.IsKeyDown(Keys.KEY_W)) input += new Vector3(0f, 0f, 1f);
                else if (Input.IsKeyDown(Keys.KEY_S)) input += new Vector3(0f, 0f, -1f);

                // Sideways movement 
                if (Input.IsKeyDown(Keys.KEY_A)) input += new Vector3(1f, 0f, 0f);
                else if (Input.IsKeyDown(Keys.KEY_D)) input += new Vector3(-1f, 0f, -0f);

                if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_RIGHT))
                {
                    dashRequest = true;
                }
            }

            if (input.SquareMagnitude() > 1f) input = input.Normalize();

            if (input != Vector3.Zero)
            {
                if (animator != null && animator.SafeToChange("Walk"))
                    animator.SetBool("Walk", true);
            }
            else
            {
                if (animator != null && animator.SafeToChange("Idle"))
                    animator.SetBool("Idle", true);
            }

            if (canInput)
            {
                if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
            }
        }

        void UpdateRotation(float dt)
        {
            // Don't manually rotate if we are dashing - dash direction should be preserved or handled by Slerp
            if (isGroundDashing || isAirDashing) return;

            if (input.SquareMagnitude() > 0.0001f)
            {
                Vector3 camForward = Vector3.Zero;
                if (camera != null)
                {
                    camForward = camera.transform.RotationQuat * Vector3.Forward;
                    camForward.y = 0f;
                    camForward = camForward.Normalize();
                }

                Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
                Vector3 moveDirInput = camForward * input.z + camRight * input.x;

                if (moveDirInput.SquareMagnitude() > 0.0001f)
                {
                    Quaternion targetRot = Quaternion.LookRotation(moveDirInput, Vector3.Up);
                    transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRot, rotSpeed * dt);
                }
            }
        }

        void HandleMovement(float dt)
        {
            Vector3 camForward = Vector3.Zero;
            if (camera != null)
            {
                camForward = camera.transform.RotationQuat * Vector3.Forward;
                camForward.y = 0f;
                camForward = camForward.Normalize();
            }

            Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
            Vector3 moveDirInput = camForward * input.z + camRight * input.x;

            if (isGroundDashing || isAirDashing)
            {
                // Dash Movement: Override horizontal velocity
                Vector3 dashVel = dashInputDir * dashSpeed;
                float yVel = isGroundDashing ? 0 : rb.Velocity.y;
                rb.Velocity = new Vector3(dashVel.x, yVel, dashVel.z);
            }
            else
            {
                // Normal Movement
                if (!isGrounded)
                {
                    Vector3 vel = rb.Velocity;
                    vel.y -= 10.0f * dt;
                    rb.Velocity = vel;
                }
                Vector3 horizontal = moveDirInput * movementSpeed;
                rb.Velocity = new Vector3(horizontal.x, rb.Velocity.y, horizontal.z);
            }
        }

        void ApplyDash()
        {
            if (dashRequest)
            {
                dashRequest = false;
                
                if (dashCooldownTimer > 0.0f) return;

                if (isGrounded)
                {
                    // Only backwards relative to facing (Backstep)
                    Vector3 backDir = -transform.Forward;
                    backDir.y = 0f;
                    dashInputDir = backDir.Normalize();

                    isGroundDashing = true;
                    dashTimer = dashDuration;
                                        
                    if (animator != null && animator.SafeToChange("DashStart"))
                        animator.SetBool("DashStart", true);
                }
                else
                {
                    //Any direction based on input
                    dashInputDir = ComputeFlatDashDir(true);

                    // Rotate immediately to dash direction for air dash
                    if (dashInputDir.SquareMagnitude() > 0.0001f)
                    {
                        transform.RotationQuat = Quaternion.LookRotation(dashInputDir, Vector3.Up);
                    }

                    isAirDashing = true;
                    dashTimer = dashDuration;
                    
                    if (animator != null && animator.SafeToChange("AirDashStart"))
                        animator.SetBool("AirDashStart", true);
                }
            }
        }

        void ApplyJump()
        {
            if (jumpRequest)
            {
                jumpRequest = false;
                if (isGrounded)
                {
                    isJumping = true;
                    rb.Velocity = new Vector3(rb.Velocity.x, 8.5f, rb.Velocity.z);
                    animator?.SetBool("JumpLoop", true);
                }
                else if (isJumping && !isDoubleJumping)
                {
                    isDoubleJumping = true;
                    rb.Velocity = new Vector3(rb.Velocity.x, 8.5f, rb.Velocity.z);
                    animator?.SetBool("AirDashStart", true);
                }
            }
        }

        Vector3 ComputeFlatDashDir(bool useMoveDir)
        {
            bool hasInput = input.SquareMagnitude() > 0.0001f;
            Vector3 normInput = hasInput ? input.Normalize() : Vector3.Zero;

            // --- NO INPUT OR FORCED FORWARD ---
            if (!useMoveDir || !hasInput)
            {
                Vector3 forward = transform.Forward;
                forward.y = 0f;
                return forward.Normalize(); 
            }

            // --- CAMERA-RELATIVE DASH ---
            if (camera != null)
            {
                Vector3 camForward = camera.transform.RotationQuat * Vector3.Forward;
                camForward.y = 0f;
                camForward = camForward.Normalize();

                Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();

                Vector3 dashDir = camForward * normInput.z + camRight * normInput.x;
                dashDir.y = 0f;
                return dashDir.Normalize();
            }

            // --- TRANSFORM-RELATIVE FALLBACK ---
            Vector3 moveDir = transform.Forward * normInput.z + transform.Right * normInput.x;
            moveDir.y = 0f;
            return moveDir.Normalize();
        }

        void TryAttack()
        {

        }

        public override void OnCollideEnter(uint other)
        {
            if (gameObject.FindGameObjectWithID(other).tag == groundName)
            {
                isGrounded = true;
                groundTriggerCount += 1;

                if (isJumping || isDoubleJumping)
                    animator?.SetBool("Land", true);

                if (isJumping) isJumping = false;
                if (isDoubleJumping) isDoubleJumping = false;
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (gameObject.FindGameObjectWithID(other).tag == groundName)
            {
                groundTriggerCount -= 1;
                groundTriggerCount = Utilities.Clamp<int>(groundTriggerCount, 0, 999);
                if (groundTriggerCount <= 0)
                {
                    isGrounded = false;
                }
            }
        }
    }
}

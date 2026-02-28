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
        public bool canInput = true;
        public bool isAttacking = false;
        public GameObject playerModel;
        public GameObject cameraObject;

        private Vector3 finalMove;
        private Vector3 input;
        private RigidBody rb;
        private Animator animator;
        private CameraController camera;
        private int groundTriggerCount = 0;
        private string groundName = "Ground";
        private bool jumpRequest = false;

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

            UpdateRotation(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            HandleMovement();
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
                else if (Input.IsKeyDown(Keys.KEY_D)) input += new Vector3(-1f, 0f, 0f);
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

        void HandleMovement()
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

            // Apply movement via Velocity instead of transform.Position to avoid stuttering
            Vector3 horizontal = moveDirInput * movementSpeed;
            rb.Velocity = new Vector3(horizontal.x, rb.Velocity.y, horizontal.z);
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
                    animator.SetBool("JumpLoop", true);

                }
                else if (isJumping && !isDoubleJumping)
                {
                    isDoubleJumping = true;
                    rb.Velocity = new Vector3(rb.Velocity.x, 8.5f, rb.Velocity.z);
                    animator.SetBool("AirDashStart", true);
                }
            }
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
                    animator.SetBool("Land", true);

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

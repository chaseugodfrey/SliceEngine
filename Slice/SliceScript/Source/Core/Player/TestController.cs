using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;


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
        public GameObject playerModel;
        public GameObject cameraObject;

        private Vector3 finalMove;
        private Vector3 input;
        private RigidBody rb;
        private Animator animator;
        private CameraController camera;
        
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
            }

            //HandleMovement(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            HandleMovement(dt);
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
                if (animator.SafeToChange("Walk"))
                    animator.SetBool("Walk", true);
            }
            else
            {
                if (animator.SafeToChange("Idle"))
                    animator.SetBool("Idle", true);
            }

            //if (Input.IsKeyPressed(Keys.KEY_SPACEBAR)) TryJump();
            if (canInput)
            {
                if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
            }
        }

        void HandleMovement(float dt)
        {
            Vector3 camForward = new Vector3();
            finalMove = Vector3.Zero;
            if (camera != null)
            {
                camForward = camera.transform.RotationQuat * Vector3.Forward; // Get camera forward direction
                camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
                camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera
            }

            Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
            Vector3 moveDirInput = camForward * input.z + camRight * input.x;
            float rawPlanarSpeed = moveDirInput.Magnitude() * movementSpeed;

            if (moveDirInput.SquareMagnitude() > 0.0001f)
            {
                Quaternion targetRot = Quaternion.LookRotation(moveDirInput, Vector3.Up);
                float scaledRotSpeed = rotSpeed;
                transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRot, scaledRotSpeed * Time.deltaTime);
            }

            Vector3 horizontal = moveDirInput * movementSpeed;
            finalMove = new Vector3(horizontal.x, rb.Velocity.y, horizontal.z);
            Console.WriteLine(finalMove.ToString());
            transform.Position += finalMove * dt;
            Console.WriteLine($"Final move is x: {transform.Position.x}, y: {transform.Position.y}, z: {transform.Position.z}");
            //rb.Velocity = finalMove;
        }

        void TryAttack()
        {

        }
    }
}

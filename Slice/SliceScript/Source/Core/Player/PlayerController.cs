using SliceEngine;
using System;
using System.Collections;


namespace SliceEngine
{

    public class PlayerController : SliceBehaviour, IInitializable
    {
        public float rotationSpeed = 50.0f;
        Animator animator;
        public string[] test3 = { "Test", "Test2" };
        public Vector3[] TestVectors = { new Vector3(1, 1, 1),  new Vector3(2, 2, 2) };
        public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);
        static bool testingShit = false;

        // =============== Movement variables =============== 
        public float moveSpeed = 2.5f;
        public float jumpForce = 5f;
        public int maxJumps = 2;
        private Coroutine groundCheckCoroutine;
        private int jumpCounter = 0;
        private Vector3 input;
        private bool canJump;

        private RigidBody rb;
        private GroundCheck groundCheck;
        private CameraController camera;

        public override void OnCreate()
        {
            Console.WriteLine("Test");
            animator = GetComponent<Animator>();
            rb = GetComponent<RigidBody>();
            if (rb == null) Console.WriteLine("No rb found");
            groundCheck = gameObject.FindGameObjectWithName("Ground Check")?.As<GroundCheck>();
        }
        
        public override void OnUpdate(float dt)
        {
            HandleInput();
            HandleMovement();
        }
        public void Initialize()
        {
            camera = Bootstrap.CameraController;
            if (camera == null)
            {
                Console.WriteLine("Camera Var in player is EMPTY");
            }

        }
        #region Movement
        private void HandleInput()
        {
            input = Vector3.Zero;
            // Forward/backward movement
            if (Input.IsKeyDown(Keys.KEY_W)) input += new Vector3(0f, 0f, 1f);
            else if (Input.IsKeyDown(Keys.KEY_S)) input += new Vector3(0f, 0f, -1f);

            // Sideways movement 
            if (Input.IsKeyDown(Keys.KEY_A)) input += new Vector3(1f, 0f, 0f);
            else if (Input.IsKeyDown(Keys.KEY_D)) input += new Vector3(-1f, 0f, 0f);

            input = input.Normalize();

            if (Input.IsKeyDown(Keys.KEY_SPACEBAR)) TryJump();
            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT)) Attack();
        }
        private void HandleMovement()
        {
            Transform camTransform;

            if (camera == null)
            {
                return;
            }
            else
            {
                camTransform = camera.transform;
            }

            Vector3 camForward = camTransform.RotationQuat * Vector3.Forward;
            camForward.y = 0f;
            camForward = camForward.Normalize();
            Vector3 moveDir = camForward * input.z + Vector3.Cross(Vector3.Up, camForward).Normalize() * input.x;
            Vector3 horizontal = moveDir * moveSpeed;

            rb.Velocity = new Vector3(horizontal.x, rb.Velocity.y, horizontal.z);
        }
        private void TryJump()
        {
            if (canJump)
            {
                jumpCounter++;
                if (jumpCounter < maxJumps) canJump = false;
                rb.AddForce(jumpForce * Vector3.Up, ForceMode.Impulse);

                Console.WriteLine("Jumps left " + (maxJumps - jumpCounter));
            }
        }
        private void ExampleMovement()
        {
            Vector3 right = Vector3.Cross(up, direction).Normalize();
            float rotationSpeedFrame = rotationSpeed * Time.deltaTime;

            if (testingShit == false)
            {
                CloneGO("GameObject_2");
                testingShit = true;
            }
            // Forwards
            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                //t.Position += direction * moveSpeed * dt;

                transform.Position += direction * moveSpeed * Time.deltaTime;

                // animator.ChangeAnim(21);
                animator.SetBool("Run", true);
                animator.SetBool("Idle", false);
                animator.SetBool("Attack", false);
            }

            // Left
            if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            {
                //t.Position -= right * moveSpeed * dt;
                Vector3 rotationAxis = new Vector3(0, 1, 0); // Y-axis
                //t.Rotate(rotationSpeedFrame, rotationAxis);
                Quaternion rotation = Quaternion.FromAxisAngle(rotationAxis.Normalize(), rotationSpeedFrame);

                this.direction = rotation * this.direction;
                this.up = rotation * this.up;


                //  animator.ChangeAnim(21);
                animator.SetBool("Run", true);
                animator.SetBool("Idle", false);
                animator.SetBool("Attack", false);
            }

            // Backward
            if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            {
                //t.Position -= direction * moveSpeed * dt;

                transform.Position -= direction * moveSpeed * Time.deltaTime;
                //    animator.ChangeAnim(21);
                animator.SetBool("Run", true);
                animator.SetBool("Idle", false);
                animator.SetBool("Attack", false);
            }

            // Right
            if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            {
                //t.Position += right * moveSpeed * dt;
                //   animator.ChangeAnim(21);
                Vector3 rotationAxis = new Vector3(0, -1, 0); // Y-axis
                //t.Rotate(rotationSpeedFrame, rotationAxis);
                Quaternion rotation = Quaternion.FromAxisAngle(rotationAxis.Normalize(), rotationSpeedFrame);

                this.direction = rotation * this.direction;
                this.up = rotation * this.up;

                animator.SetBool("Run", true);
                animator.SetBool("Attack", false);
                animator.SetBool("Idle", false);
            }

            transform.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

            if (!Input.IsKeyDown(Keys.KEY_W) && !Input.IsKeyDown(Keys.KEY_A) && !Input.IsKeyDown(Keys.KEY_S) && !Input.IsKeyDown(Keys.KEY_D))
            {
                //  animator.ChangeAnim(13);
                animator.SetBool("Idle", true);
                animator.SetBool("Attack", false);
                animator.SetBool("Run", false);
            }

            // Up (Spacebar)
            if (Input.IsKeyPressed(Keys.KEY_SPACEBAR) || Input.IsKeyDown(Keys.KEY_SPACEBAR))
            {
                transform.Position += new Vector3(0, 1, 0) * moveSpeed * Time.deltaTime;
            }


            // Scale Down
            if (Input.IsKeyDown(Keys.KEY_R) || Input.IsKeyDown(Keys.KEY_R))
            {
                animator.SetBool("Idle", false);
                animator.SetBool("Run", false);
                animator.SetBool("Attack", true);
            }
        }
        #endregion
        #region Attacks
        private void Attack()
        {
            Console.WriteLine("Attack called");
        }
        #endregion
        public override void OnCollideEnter(uint other)
        {
           // SliceLog.Log("OADMOSMODASM");
           // gameObject.Destroy();
        }
        public void OnGrounded()
        {
        }
    }
}

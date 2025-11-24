using SliceEngine;
using System;


namespace SliceEngine
{

    public class PlayerController : SliceBehaviour, IInitializable
    {
        public float rotationSpeed = 50.0f;
        Animator animator;
        Transform t;
        public string[] test3 = { "Test", "Test2" };
        public Vector3[] TestVectors = { new Vector3(1, 1, 1),  new Vector3(2, 2, 2) };
        public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);
        static bool testingShit = false;

        // =============== Movement variables =============== 
        public float moveSpeed = 2.5f;
        public float jumpHeight = 5f;
        private Vector3 input;
        private Vector3 moveDir;
        private bool canMove;
        private bool canJump;

        private CameraController camera;

        public override void OnCreate()
        {          
            t = GetComponent<Transform>();
            animator = GetComponent<Animator>();
        }

        public override void OnUpdate(float dt)
        {
            //Vector3 right = Vector3.Cross(up, direction).Normalize();
            //float rotationSpeedFrame = rotationSpeed * dt;

            //if (testingShit == false)
            //{
            //    CloneGO("GameObject_2");
            //    testingShit = true;
            //}
            //// Forwards
            //if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            //{
            //    //t.Position += direction * moveSpeed * dt;

            //    t.Position += direction * moveSpeed * dt;

            //    // animator.ChangeAnim(21);
            //    animator.SetBool("Run", true);
            //    animator.SetBool("Idle", false);
            //    animator.SetBool("Attack", false);
            //}

            //// Left
            //if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            //{
            //    //t.Position -= right * moveSpeed * dt;
            //    Vector3 rotationAxis = new Vector3(0, 1, 0); // Y-axis
            //    //t.Rotate(rotationSpeedFrame, rotationAxis);
            //    Quaternion rotation = Quaternion.FromAxisAngle(rotationAxis.Normalize(), rotationSpeedFrame);

            //    this.direction = rotation * this.direction;
            //    this.up = rotation * this.up;


            //    //  animator.ChangeAnim(21);
            //    animator.SetBool("Run", true);
            //    animator.SetBool("Idle", false);
            //    animator.SetBool("Attack", false);
            //}

            //// Backward
            //if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            //{
            //    //t.Position -= direction * moveSpeed * dt;

            //    t.Position -= direction * moveSpeed * dt;
            //    //    animator.ChangeAnim(21);
            //    animator.SetBool("Run", true);
            //    animator.SetBool("Idle", false);
            //    animator.SetBool("Attack", false);
            //}

            //// Right
            //if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            //{
            //    //t.Position += right * moveSpeed * dt;
            //    //   animator.ChangeAnim(21);
            //    Vector3 rotationAxis = new Vector3(0, -1, 0); // Y-axis
            //    //t.Rotate(rotationSpeedFrame, rotationAxis);
            //    Quaternion rotation = Quaternion.FromAxisAngle(rotationAxis.Normalize(), rotationSpeedFrame);

            //    this.direction = rotation * this.direction;
            //    this.up = rotation * this.up;

            //    animator.SetBool("Run", true);
            //    animator.SetBool("Attack", false);
            //    animator.SetBool("Idle", false);
            //}

            //t.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

            //if (!Input.IsKeyDown(Keys.KEY_W) && !Input.IsKeyDown(Keys.KEY_A) && !Input.IsKeyDown(Keys.KEY_S) && !Input.IsKeyDown(Keys.KEY_D))
            //{
            //    //  animator.ChangeAnim(13);
            //    animator.SetBool("Idle", true);
            //    animator.SetBool("Attack", false);
            //    animator.SetBool("Run", false);
            //}

            //// Up (Spacebar)
            //if (Input.IsKeyPressed(Keys.KEY_SPACEBAR) || Input.IsKeyDown(Keys.KEY_SPACEBAR))
            //{
            //    t.Position += new Vector3(0, 1, 0) * moveSpeed * dt;
            //}


            //// Scale Down
            //if (Input.IsKeyDown(Keys.KEY_R) || Input.IsKeyDown(Keys.KEY_R))
            //{
            //    animator.SetBool("Idle", false);
            //    animator.SetBool("Run", false);
            //    animator.SetBool("Attack", true);
            //}
            HandleInput();
            HandleMovement(dt);
        }
        public void Initialize()
        {
            camera = Bootstrap.CameraController;
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

            if (Input.IsKeyDown(Keys.KEY_SPACEBAR)) input += new Vector3(0f, jumpHeight, 0f);
        }
        private void HandleMovement(float dt)
        {
            Transform camTransform = camera.transform;
            Vector3 camForward = camTransform.RotationQuat * Vector3.Forward;
            camForward.y = 0f;
            camForward = camForward.Normalize();
            Vector3 moveDir = camForward * input.z + Vector3.Cross(Vector3.Up, camForward).Normalize() * input.x;

            Vector3 speed = moveDir * moveSpeed;
            t.Position += new Vector3(speed.x, speed.y, speed.z) * dt;
        }
        #endregion
        public override void OnCollideEnter(uint other)
        {
           // SliceLog.Log("OADMOSMODASM");
           // gameObject.Destroy();
        }
    }
}

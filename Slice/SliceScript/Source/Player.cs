using SliceEngine;
using System;


namespace SliceEngine
{

    public class Player : SliceBehaviour
    {
        public float moveSpeed = 2.5f;
        public float rotationSpeed = 5.0f;
        Animator animator;
        Transform t;
        public string[] test3 = { "Test", "Test2" };
        public Vector3[] TestVectors = { new Vector3(1, 1, 1),  new Vector3(2, 2, 2) };
        public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 camera  = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);

        public override void OnCreate()
        {
            t = GetComponent<Transform>();
            animator = GetComponent<Animator>();

        }

        public override void OnUpdate(float dt)
        { 
            Vector3 right = Vector3.Cross(up, camera).Normalize();
            Vector3 rotationAxis = new Vector3(0, 1, 0);
            Vector3 targetFacingDirection = this.direction;
            bool isMoving = false;

            // Forwards
            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                t.Position += camera * moveSpeed * dt;
                targetFacingDirection = camera;
                isMoving = true;
            }

            // Left
            if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            {
                t.Position += right * moveSpeed * dt;
                targetFacingDirection = right;
                isMoving = true;
            }

            // Backward
            if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            { 
                t.Position -= camera * moveSpeed * dt;
                targetFacingDirection = new Vector3(-camera.x,-camera.y,-camera.z);
                isMoving = true;

            }

            // Right
            if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            {
                t.Position -= right * moveSpeed * dt;
                targetFacingDirection = new Vector3(-right.x, -right.y, -right.z);
                isMoving = true;
            }

            if (targetFacingDirection.LengthSquared() > 0.001f)
            {
                this.direction = Vector3.Slerp(
                    this.direction,
                    targetFacingDirection.Normalize(),
                    rotationSpeed * dt
                );
            }

            t.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

            if (isMoving)
            {
                animator.SetBool("Run", true);
                animator.SetBool("Idle", false);
                animator.SetBool("Attack", false);
            }
            else
            {
                animator.SetBool("Run", false);
                animator.SetBool("Idle", true);
                animator.SetBool("Attack", false);
            }

            // Up (Spacebar)
            if (Input.IsKeyPressed(Keys.KEY_SPACEBAR) || Input.IsKeyDown(Keys.KEY_SPACEBAR))
            {
                t.Position += new Vector3(0, 1, 0) * moveSpeed * dt;
            }


            // Scale Down
            if (Input.IsKeyDown(Keys.KEY_R) || Input.IsKeyDown(Keys.KEY_R))
            {
                animator.SetBool("Idle", false);
                animator.SetBool("Run", false);
                animator.SetBool("Attack", true);
            }
        }

        public override void OnCollideEnter(uint other)
        {
           // SliceLog.Log("OADMOSMODASM");
           // gameObject.Destroy();
        }
    }
}

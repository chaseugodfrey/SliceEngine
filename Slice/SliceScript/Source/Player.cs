using SliceEngine;
using System;


namespace SliceEngine
{

    public class Player : SliceBehaviour
    {
        public float moveSpeed = 2.5f;
        public float rotationSpeed = 50.0f;
        Animator animator;
        Transform t;

        public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);

        public override void OnCreate()
        {
            t = GetComponent<Transform>();
            animator = GetComponent<Animator>();

        }

        public override void OnUpdate(float dt)
        { 
            //Vector3 right = Vector3.Cross(up, direction).Normalize();
            float rotationSpeedFrame = rotationSpeed * dt;


            // Forwards
            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                //t.Position += direction * moveSpeed * dt;

                t.Position += direction * moveSpeed * dt;

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

                t.Position -= direction * moveSpeed * dt;
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

            t.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

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
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestTransforms : SliceBehaviour
    {
        public float moveSpeed = 5.0f;
        public float rotationSpeed = 25.0f;
        public float scaleSpeed = 0.5f;

        Transform t;
        Animator animator;

        Vector3 direction = new Vector3(-1.0f, 0.0f, 0.0f);
        Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);

        public override void OnCreate()
        {
            t = GetComponent<Transform>();
            animator = GetComponent<Animator>();
            animator.ChangeAnim(13);
        }

        public override void OnUpdate(float dt)
        {
            Vector3 right = Vector3.Cross(up, direction).Normalize();

            // Forwards
            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                t.Position += direction * moveSpeed * dt;
                animator.ChangeAnim(21);
            }

            // Left
            if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            {
                t.Position -= right * moveSpeed * dt;
                animator.ChangeAnim(21);
            }

            // Backward
            if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            {
                t.Position -= direction * moveSpeed * dt;
                animator.ChangeAnim(21);
            }

            // Right
            if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            {
                t.Position += right * moveSpeed * dt;
                animator.ChangeAnim(21);
            }

            if (!Input.IsKeyDown(Keys.KEY_W) && !Input.IsKeyDown(Keys.KEY_A) && !Input.IsKeyDown(Keys.KEY_S) && !Input.IsKeyDown(Keys.KEY_D))
            {
                animator.ChangeAnim(13);
            }

            // Scale Up
            if (Input.IsKeyPressed(Keys.KEY_Q) || Input.IsKeyDown(Keys.KEY_Q))
            {
                t.Scale += new Vector3(scaleSpeed, scaleSpeed, scaleSpeed) * dt;
            }

            // Scale Down
            if (Input.IsKeyPressed(Keys.KEY_E) || Input.IsKeyDown(Keys.KEY_E))
            {
                t.Scale -= new Vector3(scaleSpeed, scaleSpeed, scaleSpeed) * dt;
            }            

            if (Input.IsKeyPressed(Keys.KEY_Z) || Input.IsKeyDown(Keys.KEY_Z))
            {
                float rotationSpeedFrame = rotationSpeed * dt;
                t.Rotate(rotationSpeedFrame, new Vector3(1, 0, 0)); // X-axis pitch
            }

            if (Input.IsKeyPressed(Keys.KEY_X) || Input.IsKeyDown(Keys.KEY_X))
            {
                float rotationSpeedFrame = rotationSpeed * dt;
                t.Rotate(rotationSpeedFrame, new Vector3(0, 1, 0)); // Y-axis yaw
            }

            if (Input.IsKeyPressed(Keys.KEY_C) || Input.IsKeyDown(Keys.KEY_C))
            {
                float rotationSpeedFrame = rotationSpeed * dt;
                t.Rotate(rotationSpeedFrame, new Vector3(0, 0, 1)); // Z-axis roll
            }
        }

    }
}

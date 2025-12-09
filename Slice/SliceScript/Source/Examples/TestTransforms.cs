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
           // t = GetComponent<Transform>();
           animator = GetComponent<Animator>();


            SliceLog.Log("Rotation: x<" + t.Rotation.x + ">y<" + t.Rotation.y + ">z<" + t.Rotation.z);
        }

        public override void OnUpdate(float dt)
        {
            Vector3 right = Vector3.Cross(up, direction).Normalize();

            // Forwards
            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                t.Position += direction * moveSpeed * dt;

                // animator.ChangeAnim(21);
                //animator.SetBool("Run", true);
                //animator.SetBool("Idle", false);
                //animator.SetBool("Attack", false);
            }

            // Left
            if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            {
                t.Position -= right * moveSpeed * dt;
                //  animator.ChangeAnim(21);
                //animator.SetBool("Run", true);
                //animator.SetBool("Idle", false);
                //animator.SetBool("Attack", false);
            }

            // Backward
            if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            {
                t.Position -= direction * moveSpeed * dt;
                //    animator.ChangeAnim(21);
                //animator.SetBool("Run", true);
                //animator.SetBool("Idle", false);
                //animator.SetBool("Attack", false);
            }

            // Right
            if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            {
                t.Position += right * moveSpeed * dt;
                //   animator.ChangeAnim(21);
                //animator.SetBool("Run", true);
                //animator.SetBool("Attack", false);
                //animator.SetBool("Idle", false);
            }

            if (!Input.IsKeyDown(Keys.KEY_W) && !Input.IsKeyDown(Keys.KEY_A) && !Input.IsKeyDown(Keys.KEY_S) && !Input.IsKeyDown(Keys.KEY_D))
            {
                //  animator.ChangeAnim(13);
                //animator.SetBool("Idle", true);
                //animator.SetBool("Attack", false);
                //animator.SetBool("Run", false);
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

            // Down (Ctrl)
            if (Input.IsKeyPressed(Keys.KEY_3) || Input.IsKeyDown(Keys.KEY_3))
            {
                t.Position -= new Vector3(0, 1, 0) * moveSpeed * dt;
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

            float rotationSpeedFrame = rotationSpeed * dt;

            if (Input.IsKeyPressed(Keys.KEY_Z) || Input.IsKeyDown(Keys.KEY_Z)) t.Rotate(rotationSpeedFrame, new Vector3(1, 0, 0));
            if (Input.IsKeyPressed(Keys.KEY_X) || Input.IsKeyDown(Keys.KEY_X)) t.Rotate(rotationSpeedFrame, new Vector3(0, 1, 0));
            if (Input.IsKeyPressed(Keys.KEY_C) || Input.IsKeyDown(Keys.KEY_C)) t.Rotate(rotationSpeedFrame, new Vector3(0, 0, 1));
        }

        public void LaunchParabola(Transform target, Vector3 start, Vector3 end, float height, float duration)
        {
            CoroutineManager.StartCoroutine(Utilities.ParabolaCoroutine(target, start, end, height, duration), this);
        }
    }
}

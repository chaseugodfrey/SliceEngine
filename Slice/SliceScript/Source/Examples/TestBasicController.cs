using SliceEngine;
using System;


namespace SliceEngine
{

    public class TestBasicController : SliceBehaviour
    {
        public float moveSpeed = 2.5f;
        public float rotationSpeed = 250.0f;
        Animator animator;
        Transform t;
        GameObject floor;

        public int CameraID = 14;
        public string[] test3 = { "Test", "Test2" };
        public Vector3[] TestVectors = { new Vector3(1, 1, 1), new Vector3(2, 2, 2) };
        public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 camera = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);

        //remove these when testingShit gets used
#pragma warning disable 0414
        static bool testingShit = false;
#pragma warning restore 0414

        float timeBuffer = 0.0f;
        float attackBuffer = 0.0f;
        bool startAttack = false;
        bool startBuffer = false;
        bool grounded = false;
        int jumpCounter = 0;

        public override void OnCreate()
        {
            t = GetComponent<Transform>();
            animator = GetComponent<Animator>();
            floor = gameObject.FindGameObjectWithName("FloorQuad");
        }

        public override void OnUpdate(float dt)
        {
            Vector3 right = Vector3.Cross(camera, up).Normalize();
            Vector3 front = Vector3.Cross(up, right).Normalize();
            Vector3 rotationAxis = new Vector3(0, 1, 0);
            Vector3 targetFacingDirection = this.direction;
            float rotationSpeedFrame = rotationSpeed * dt;

            if (Input.IsKeyPressed(Keys.KEY_K))
            {
                Console.WriteLine("PRESSING THE K KEY");
                GameObject GO = gameObject.FindGameObjectWithID((uint)CameraID);
                Camera.SetMainCamera(GO);
            }

            if (Input.IsKeyPressed(Keys.KEY_L))
            {
                Console.WriteLine("PRESSING THE L KEY");
                GameObject GO = FindGameObjectWithName("Camera_1");
                Camera.SetMainCamera(GO);
            }

            //if (testingShit == false)
            //{
            //    CloneGO("GameObject_2");
            //    testingShit = true;
            //}
            // Forwards
            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                t.Position = t.Position + front * moveSpeed * dt;
                targetFacingDirection = front;
                //if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0)
                    animator.SetBool("Walk", true);
            }

            // Left
            if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            {
                t.Position = t.Position - right * moveSpeed * dt;
                targetFacingDirection = new Vector3(-right.x, -right.y, -right.z);
                //if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0)
                    animator.SetBool("Walk", true);
            }

            // Backward
            if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            {
                t.Position = t.Position - front * moveSpeed * dt;
                targetFacingDirection = new Vector3(-front.x, -front.y, -front.z);

                //if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0)
                    animator.SetBool("Walk", true);

            }

            // Right
            if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            {
                t.Position = t.Position + right * moveSpeed * dt;
                targetFacingDirection = right;
                //if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0)
                    animator.SetBool("Walk", true);
            }

            //Quaternion dir = Quaternion.LookRotation(this.direction, this.up);
            //if (targetFacingDirection.LengthSquared() > 0.001f)
            {

                //Quaternion target = Quaternion.LookRotation(targetFacingDirection, this.up);

                // dir = Quaternion.RotateTowards(dir, target, rotationSpeed * dt);

                this.direction = Vector3.RotateTowards(this.direction, targetFacingDirection.Normalize(), rotationSpeedFrame);
            }

            t.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

            if (!Input.IsKeyDown(Keys.KEY_W) && !Input.IsKeyDown(Keys.KEY_A) && !Input.IsKeyDown(Keys.KEY_S) && !Input.IsKeyDown(Keys.KEY_D))
            {
                //if (String.Compare(animator.GetCurrAnimName(), "Walk") == 0 ||
                //    String.Compare(animator.GetCurrAnimName(), "Attack1") == 0 ||
                //    String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                    animator.SetBool("Idle", true);
            }

            // Up (Spacebar)
            if (Input.IsKeyPressed(Keys.KEY_SPACEBAR) || Input.IsKeyDown(Keys.KEY_SPACEBAR))
            {
                // idk why this shit kinda buggy when i jump 2 times
                // is i fail maths or maths fail me
                if (jumpCounter < 2)
                {
                    //if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
                    //    String.Compare(animator.GetCurrAnimName(), "Walk") == 0 ||
                    //    String.Compare(animator.GetCurrAnimName(), "Attack1") == 0 ||
                    //    String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                    {
                        animator.SetBool("JumpLoop", true);
                        t.Position += new Vector3(0, 5, 0);
                        if (jumpCounter == 0)
                        {
                            AudioSettings.PlaySFX("Jump");
                        }
                        else if (jumpCounter == 1)
                        {
                            //Put it as more than once cause idk why when its == 1 the jump and the double jump plays at the same time
                            if (String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
                                String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                            {
                                animator.SetBool("AirDashStart", true);
                            }
                            AudioSettings.PlaySFX("DoubleJump");
                        }
                        jumpCounter++;

                    }
                }
            }

            if (!grounded)
            {
                if (String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "AirDashStart") == 0)
                {
                    animator.SetBool("Plunge", true);
                }
            }

            // Scale Down
            if (Input.IsKeyPressed(Keys.KEY_R))// || Input.IsKeyDown(Keys.KEY_R))
            {
                //if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 || String.Compare(animator.GetCurrAnimName(), "Walk") == 0)
                {
                    animator.SetBool("Attack1", true);
                    AudioSettings.PlaySFX("A1");
                }

                startBuffer = true;
                startAttack = true;
                timeBuffer = 0.0f;
            }
        }

        public override void OnCollideEnter(uint other)
        {
            if (other == floor.mID)
            {
                grounded = true;
                jumpCounter = 0;
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (other == floor.mID)
            {
                grounded = false;
            }
        }
    }
}

using SliceEngine;
using System;


namespace SliceEngine
{

    public class Player : SliceBehaviour
    {
        public float moveSpeed = 2.5f;
        public float rotationSpeed = 20.0f;
        Animator animator;
        Transform t;
        public string[] test3 = { "Test", "Test2" };
        public Vector3[] TestVectors = { new Vector3(1, 1, 1),  new Vector3(2, 2, 2) };
        public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 camera  = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);
        static bool testingShit = false;

        int moveCounter = 0;
        float timeBuffer = 0.0f;
        bool startBuffer = false;

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
            //Vector3 right = Vector3.Cross(up, direction).Normalize();
            float rotationSpeedFrame = rotationSpeed * dt;

            

            if (testingShit == false)
            {
                CloneGO("GameObject_2");
                testingShit = true;
            }
            // Forwards
            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                t.Position += camera * moveSpeed * dt;
                targetFacingDirection = camera;
                Console.WriteLine("Name: " + animator.GetCurrAnimName());
                if (String.Compare(animator.GetCurrAnimName(), "player|Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|Attack3ToLoco") == 0)
                    animator.SetBool("player|Walk", true);
            }

            // Left
            if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            {
                t.Position += right * moveSpeed * dt;
                targetFacingDirection = right;
                if (String.Compare(animator.GetCurrAnimName(), "player|Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|Attack3ToLoco") == 0)
                    animator.SetBool("player|Walk", true);
            }

            // Backward
            if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            { 
                t.Position -= camera * moveSpeed * dt;
                targetFacingDirection = new Vector3(-camera.x,-camera.y,-camera.z);
                if (String.Compare(animator.GetCurrAnimName(), "player|Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|Attack3ToLoco") == 0)
                    animator.SetBool("player|Walk", true);

            }

            // Right
            if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            {
                t.Position -= right * moveSpeed * dt;
                targetFacingDirection = new Vector3(-right.x, -right.y, -right.z);
                if (String.Compare(animator.GetCurrAnimName(), "player|Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "player|Attack3ToLoco") == 0)
                    animator.SetBool("player|Walk", true);
            }

            //Quaternion dir = Quaternion.LookRotation(this.direction, this.up);
            if (targetFacingDirection.LengthSquared() > 0.001f)
            {

                //Quaternion target = Quaternion.LookRotation(targetFacingDirection, this.up);

               // dir = Quaternion.RotateTowards(dir, target, rotationSpeed * dt);

                this.direction = Vector3.RotateTowards(this.direction,targetFacingDirection.Normalize(),rotationSpeed * dt);
            }

            t.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

            if (!Input.IsKeyDown(Keys.KEY_W) && !Input.IsKeyDown(Keys.KEY_A) && !Input.IsKeyDown(Keys.KEY_S) && !Input.IsKeyDown(Keys.KEY_D))
            {
                if (String.Compare(animator.GetCurrAnimName(), "player|Walk") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle1") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "player|AttackToIdle2") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "player|Attack3ToLoco") == 0   )
                    animator.SetBool("player|Idle", true);
            }

            // Up (Spacebar)
            if (Input.IsKeyPressed(Keys.KEY_SPACEBAR) || Input.IsKeyDown(Keys.KEY_SPACEBAR))
            {
                t.Position += new Vector3(0, 1, 0) * moveSpeed * dt;
            }


            // Scale Down
            if (Input.IsKeyPressed(Keys.KEY_R))// || Input.IsKeyDown(Keys.KEY_R))
            {
                if (String.Compare(animator.GetCurrAnimName(), "player|Idle") == 0 || String.Compare(animator.GetCurrAnimName(), "player|Walk") == 0)
                {
                    animator.SetBool("player|Attack1", true);
                }

                if (String.Compare(animator.GetCurrAnimName(), "player|Attack1") == 0)
                {
                    animator.SetBool("player|Attack2", true);
                }
                if (String.Compare(animator.GetCurrAnimName(), "player|Attack2") == 0)
                {
                    animator.SetBool("player|Attack3", true);
                }

                startBuffer = true;
                timeBuffer = 0.0f;

            }


            // duble bifferb for attack
            if(startBuffer)
            {
                timeBuffer += dt;
                
                if (timeBuffer > 0.5f)
                {
                    if (String.Compare(animator.GetCurrAnimName(), "player|Attack1") == 0)
                    {
                        animator.SetBool("player|AttackToIdle1", true);
                    }
                    if(String.Compare(animator.GetCurrAnimName(), "player|Attack2") == 0)
                    {
                        animator.SetBool("player|AttackToIdle2", true);
                    }
                    if (String.Compare(animator.GetCurrAnimName(), "player|Attack3") == 0)
                    {
                        animator.SetBool("player|Attack3ToLoco", true);
                    }
                    startBuffer = false;
                    timeBuffer = 0.0f;
                }
            }
            
            


        }

        public override void OnCollideEnter(uint other)
        {
           // SliceLog.Log("OADMOSMODASM");
           // gameObject.Destroy();
        }
    }
}

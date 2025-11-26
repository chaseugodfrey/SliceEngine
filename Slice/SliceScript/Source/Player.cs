using SliceEngine;
using System;


namespace SliceEngine
{

    public class Player : SliceBehaviour
    {
        public float moveSpeed = 2.5f;
        public float rotationSpeed = 250.0f;
        Animator animator;
        Transform t;
        GameObject floor;
        ColliderShape Attack_Collider_1;


        public string[] test3 = { "Test", "Test2" };
        public Vector3[] TestVectors = { new Vector3(1, 1, 1),  new Vector3(2, 2, 2) };
        public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        public Vector3 camera  = new Vector3(0.0f, 0.0f, 1.0f);
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
            Attack_Collider_1 = gameObject.FindGameObjectWithName("Attack_Collider_1").GetComponent<ColliderShape>();
            Attack_Collider_1.ComponentEnabled = false;
            Console.WriteLine("ALOYSISU LOOK HERE<" + Attack_Collider_1.gameObject.mID + ">");
        }

        public override void OnUpdate(float dt)
        { 
            Vector3 right = Vector3.Cross(camera, up).Normalize();
            Vector3 front = Vector3.Cross(up, right).Normalize();
            Vector3 rotationAxis = new Vector3(0, 1, 0);
            Vector3 targetFacingDirection = this.direction;
            float rotationSpeedFrame = rotationSpeed * dt;

            

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
                if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "PlungeToWalk") == 0)
                    animator.SetBool("Walk", true);

                if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") == 0)
                {
                    animator.SetBool("PlungeToWalk", true);
                }


                //Console.WriteLine("Name: " + animator.GetCurrAnimName());

            }

            // Left
            if (Input.IsKeyPressed(Keys.KEY_A) || Input.IsKeyDown(Keys.KEY_A))
            {
                t.Position = t.Position - right * moveSpeed * dt;
                targetFacingDirection = new Vector3(-right.x, -right.y, -right.z);
                if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "PlungeToWalk") == 0)
                    animator.SetBool("Walk", true);

                if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") == 0)
                {
                    animator.SetBool("PlungeToWalk", true);
                }
            }

            // Backward
            if (Input.IsKeyPressed(Keys.KEY_S) || Input.IsKeyDown(Keys.KEY_S))
            {
                t.Position = t.Position - front * moveSpeed * dt;
                targetFacingDirection = new Vector3(-front.x, -front.y, -front.z);

                if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "PlungeToWalk") == 0)
                    animator.SetBool("Walk", true);

                if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") == 0)
                {
                    animator.SetBool("PlungeToWalk", true);
                }

            }

            // Right
            if (Input.IsKeyPressed(Keys.KEY_D) || Input.IsKeyDown(Keys.KEY_D))
            {
                t.Position = t.Position + right * moveSpeed * dt;
                targetFacingDirection = right;
                if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                   String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "PlungeToWalk") == 0)
                    animator.SetBool("Walk", true);

                if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") == 0)
                {
                    animator.SetBool("PlungeToWalk", true);
                }
            }

            //Quaternion dir = Quaternion.LookRotation(this.direction, this.up);
            //if (targetFacingDirection.LengthSquared() > 0.001f)
            {

                //Quaternion target = Quaternion.LookRotation(targetFacingDirection, this.up);

               // dir = Quaternion.RotateTowards(dir, target, rotationSpeed * dt);

                this.direction = Vector3.RotateTowards(this.direction,targetFacingDirection.Normalize(), rotationSpeedFrame);
            }

            t.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

            if (!Input.IsKeyDown(Keys.KEY_W) && !Input.IsKeyDown(Keys.KEY_A) && !Input.IsKeyDown(Keys.KEY_S) && !Input.IsKeyDown(Keys.KEY_D))
            {
                if (String.Compare(animator.GetCurrAnimName(), "Walk") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "PlungeToIdle") == 0)
                    animator.SetBool("Idle", true);

                if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") == 0)
                {
                    animator.SetBool("PlungeToIdle", true);
                }
            }

            // Up (Spacebar)
            if (Input.IsKeyPressed(Keys.KEY_SPACEBAR) || Input.IsKeyDown(Keys.KEY_SPACEBAR))
            {
         
                if(jumpCounter < 2)
               { 
                    if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
                        String.Compare(animator.GetCurrAnimName(), "Walk") == 0 ||
                        String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                        String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                        String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                        String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                    {
                        animator.SetBool("JumpLoop", true);
                        t.Position += new Vector3(0, 5, 0);
                        if(jumpCounter == 0)
                        {
                            AudioSettings.PlaySFX("Jump");
                        }
                        if(jumpCounter == 1)
                        {
                            AudioSettings.PlaySFX("DoubleJump");
                        }
                        jumpCounter++;
                        
                    }
                }
            }

            if(!grounded)
            {
                if (String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0)
                {
                    animator.SetBool("Plunge", true);
                }
            }

            if (grounded)
            {

                if (String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                {
                    animator.SetBool("PlungeLand", true);
                    AudioSettings.PlaySFX("Land");
                }
            }



            // Scale Down
            if (Input.IsKeyPressed(Keys.KEY_R))// || Input.IsKeyDown(Keys.KEY_R))
            {
                if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 || String.Compare(animator.GetCurrAnimName(), "Walk") == 0)
                {
                    animator.SetBool("Attack1", true);
                    Attack_Collider_1.ComponentEnabled = true;
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                {
                    animator.SetBool("Attack2", true);
                    Attack_Collider_1.ComponentEnabled = true;
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                {
                    animator.SetBool("Attack3", true);
                    Attack_Collider_1.ComponentEnabled = true;
                }

                startBuffer = true;
                startAttack = true;
                timeBuffer = 0.0f;
            }


            // duble bifferb for attack
            if(startBuffer)
            {
                timeBuffer += dt;
                
                if (timeBuffer > 0.5f)
                {
                    if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                    {
                        animator.SetBool("AttackToIdle1", true);
                        Attack_Collider_1.ComponentEnabled = false;
                    }
                    if(String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                    {
                        animator.SetBool("AttackToIdle2", true);
                        Attack_Collider_1.ComponentEnabled = false;
                    }
                    if (String.Compare(animator.GetCurrAnimName(), "Attack3") == 0)
                    {
                        animator.SetBool("Attack3ToLoco", true);
                    }

                    startBuffer = false;
                    timeBuffer = 0.0f;
                }
            }

            if(startAttack) 
            {
                attackBuffer += dt;
                if(attackBuffer > 1.0f)
                {
                    startAttack = false;
                    attackBuffer = 0.0f;
                    Attack_Collider_1.ComponentEnabled = false;
                }
            }


            //Console.WriteLine("anime time here in player.cs line 242 : " + animator.GetCurrAnimTime().ToString());
        }

        public override void OnCollideEnter(uint other)
        {
            if(other == floor.mID)
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

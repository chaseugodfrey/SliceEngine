using SliceEngine;
using System;


namespace SliceEngine
{

    public class Old_Test_Player : SliceBehaviour
    {
        public float moveSpeed = 2.5f;
        public float rotationSpeed = 250.0f;
        Animator animator;
        Transform t;
        GameObject floor;
        ColliderShape Attack_Collider_1;
        AudioSource myAudio;

        public int CameraID = 14;
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

        GameObject enemy;
        RigidBody enemyRb;
        bool isEnemyKnockedBack = false;
        float knockbackTimer = 0.0f;

        public override void OnCreate()
        {
            t = GetComponent<Transform>();
            animator = GetComponent<Animator>();
            myAudio = GetComponent<AudioSource>();
            floor = FindGameObjectWithName("Floor");
            if (floor == null) FunctionCalls.LogWarn("PlayerTest: 'Floor' not found!");

            enemy = FindGameObjectWithName("Enemy");
            if (enemy != null)
            {
                enemyRb = enemy.GetComponent<RigidBody>();
            }
            else
            {
                FunctionCalls.LogWarn("PlayerTest: 'Enemy' not found!");
            }
            Attack_Collider_1 = gameObject.FindGameObjectWithName("Attack_Collider_1").GetComponent<ColliderShape>();
            Attack_Collider_1.ComponentEnabled = false;
            //Console.WriteLine("ALOYSISU LOOK HERE<" + Attack_Collider_1.gameObject.mID + ">");
        }

        public override void OnUpdate(float dt)
        { 
            Vector3 right = Vector3.Cross(camera, up).Normalize();
            Vector3 front = Vector3.Cross(up, right).Normalize();
            Vector3 rotationAxis = new Vector3(0, 1, 0);
            Vector3 targetFacingDirection = this.direction;
            float rotationSpeedFrame = rotationSpeed * dt;

            if(Input.IsKeyPressed(Keys.KEY_K))
            {
                //Console.WriteLine("PRESSING THE K KEY");
                GameObject GO = gameObject.FindGameObjectWithID((uint)CameraID);
                Camera.SetMainCamera(GO);
            }

            if (Input.IsKeyPressed(Keys.KEY_L))
            {
                //Console.WriteLine("PRESSING THE L KEY");
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
                // idk why this shit kinda buggy when i jump 2 times
                // is i fail maths or maths fail me
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
                        else if(jumpCounter == 1)
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

            if(!grounded)
            {
                if (String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "AirDashStart") == 0)
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
                    AudioSettings.PlaySFX("A1");
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                {
                    animator.SetBool("Attack2", true);
                    Attack_Collider_1.ComponentEnabled = true;
                    AudioSettings.PlaySFX("A2");
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

            //Need to find another way to play the walking
            if (animator.GetCurrAnimName() == "Walk")
            {
                myAudio.Play();
            }
            else
            {
                myAudio.Stop();
            }

            // duble bifferb for attack
            if (startBuffer)
            {
                timeBuffer += dt;

                if (timeBuffer > 0.5f)
                {
                    if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                    {
                        animator.SetBool("AttackToIdle1", true);
                        Attack_Collider_1.ComponentEnabled = false;
                    }
                    if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                    {
                        animator.SetBool("AttackToIdle2", true);
                        Attack_Collider_1.ComponentEnabled = false;
                    }
                    if (String.Compare(animator.GetCurrAnimName(), "Attack3") == 0)
                    {
                        animator.SetBool("Attack3ToLoco", true);
                        AudioSettings.PlaySFX("A3");
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

            // Monitor Enemy Landing Logic
/*            if (isEnemyKnockedBack && enemyRb != null && enemyAgent != null)
            {
                knockbackTimer += dt;

                // Wait a bit before checking for landing (to let it fly up first)
                if (knockbackTimer > 0.5f)
                {
                    // Simple landing check: Is velocity low? Is it close to Y=0 (or whatever floor height is)?
                    // Or relies on the enemy's own collision logic if available.
                    // For now, let's just use a timer + height check as a proxy for "Landed"

                    float enemyY = enemy.GetComponent<Transform>().Position.y;

                    // If enemy is falling/on ground (approx 0.5 height)
                    if (enemyY < 1.0f)
                    {
                        // Re-enable Navigation
                        enemyAgent.SetComponentIsEnabled(enemy, true);
                        isEnemyKnockedBack = false;
                        knockbackTimer = 0.0f;
                        FunctionCalls.Log("Enemy Landed: Resuming Navigation");
                    }
                }
            }*/
        }

        public override void OnCollideEnter(uint other)
        {
            if (enemy != null && other == enemy.mID)
            {
/*                FunctionCalls.Log("PlayerTest: OnCollideEnter");
                if (enemyAgent != null && enemyAgent.ComponentIsEnabled(enemy))
                {
                    enemyAgent.SetComponentIsEnabled(enemy, false);
                    isEnemyKnockedBack = true;
                    knockbackTimer = 0.0f; // Reset timer
                }
                if (enemyRb != null)
                {
                    Vector3 force = new Vector3(5.0f, 8.0f, 0.0f);
                    enemyRb.AddForce(force, ForceMode.Impulse);
                    FunctionCalls.Log("Collision with Enemy: Applied Knockback (Nav Disabled)");
                }*/
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

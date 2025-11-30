using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;


namespace SliceEngine
{

    public class PlayerController : Entity, IInitializable
    {
        //public float rotationSpeed = 50.0f;
        //public string[] test3 = { "Test", "Test2" };
        //public Vector3[] TestVectors = { new Vector3(1, 1, 1),  new Vector3(2, 2, 2) };
        //public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        //public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);
        //static bool testingShit = false;

        public GameObject playerModel;

        // =============== Movement variables =============== 
        public float moveSpeed = 2.5f;
        public float dashMultiplier = 5f;
        public float dashDuration = 0.2f;
        private bool isDashing = false;
        public float jumpForce = 5f;
        public float fallForce = 5f;
        public float fallVelocityThreshold = 20f;
        public int maxJumps = 2;
        public float groundCheckDelay = 0.05f;
        private float groundCheckTimer = 0f;
        private bool grounded, groundCheckLocked;
        private int jumpCounter = 0;
        private Vector3 input;

        // =============== Attack variables =============== 
        public float attackResetTime = 1f;
        private float attackResetTimer = 0f;
        private int attackCounter = 0;
        private bool isAttacking = false;

        public string attack1HBName;
        public int attack1Damage;
        public Vector3 attack1Window; 
        public float attack1Duration;
        private Hitbox attack1HB; 

        public string attack2HBName;
        public int attack2Damage;
        public Vector3 attack2Window;
        public float attack2Duration;
        private Hitbox attack2HB;

        public string attack3HBName;
        public int attack3Damage;
        public Vector3 attack3Window;
        public float attack3Duration;
        private Hitbox attack3HB;

        // =============== Internal variables =============== 
        public bool canInput = false;
        public bool canMove = true;
        private RigidBody rb;
        private GroundCheck groundCheck;
        private Animator animator;
        private CameraController camera;
        private AudioSource audio;

        public void Initialize()
        {
            camera = Bootstrap.CameraController;
            if (camera == null)
            {
                Console.WriteLine("Camera Var in player is EMPTY");
            }
        }
        public override void OnCreate()
        {
            Console.WriteLine("Test");
            playerModel = gameObject.FindGameObjectWithName("RootNode");
            animator = playerModel?.GetComponent<Animator>();
            if (animator == null) Console.WriteLine("No animator found");
            else Console.WriteLine("Animator found");
            audio = gameObject.GetComponent<AudioSource>();
            rb = GetComponent<RigidBody>();
            if (rb == null) Console.WriteLine("No rb found");
            else Console.WriteLine("RB found");
            groundCheck = gameObject.FindGameObjectWithName("Ground Check")?.As<GroundCheck>();
            if (groundCheck == null) Console.WriteLine("No ground check found");

            InitializeAttackHitboxes();
        }
        
        public override void OnUpdate(float dt)
        {
            GroundCheck();
            if (canInput) HandleInput();
            if (canMove) HandleMovement();
            AttackResetTimer();
        }
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

            if (input != Vector3.Zero)
            {
                animator.ChangeAnim(21);
                animator.SetBool("Walk", true);
            }
            //else
            //{
            //    animator.SetBool("Idle", true);
            //}

            if (Input.IsKeyPressed(Keys.KEY_SPACEBAR)) TryJump();
            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
        }
        #region Movement
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

            Vector3 camForward = camTransform.RotationQuat * Vector3.Forward; // Get camera forward direction
            camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
            camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera
            Vector3 moveDir = camForward * input.z + Vector3.Cross(Vector3.Up, camForward).Normalize() * input.x;

            if (isDashing) return;

            // Handle regular movement
            Vector3 movement = moveDir * moveSpeed;
            bool falling = rb.Velocity.y < fallVelocityThreshold && !grounded;
            if (falling) rb.AddForce(Vector3.Down * fallForce, ForceMode.Impulse);

            rb.Velocity = new Vector3(movement.x, rb.Velocity.y, movement.z);

            if (moveDir.LengthSquared() > 0.01f)
            {
                playerModel.GetComponent<Transform>().RotationQuat = Quaternion.LookRotation(moveDir, Vector3.Up);
            }

            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_RIGHT))
            {
                if (!isDashing) StartCoroutine(Dash(moveDir));
            }


            //Console.WriteLine(animator.GetCurrAnimName());
        }
        private void TryJump()
        {
            if (CanJump())
            {

                rb.Velocity = new Vector3(rb.Velocity.x, 0f, rb.Velocity.z); // Reset vertical velocity before applying jump force
                jumpCounter++;

                if (jumpCounter == 1)
                {
                    if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
                            String.Compare(animator.GetCurrAnimName(), "Walk") == 0 ||
                            String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                            String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                            String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                            String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                        animator.SetBool("JumpLoop", true);
                    AudioSettings.PlaySFX("Jump");
                }
                else if (jumpCounter == 2)
                {
                    if (String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
                                String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                    {
                        animator.SetBool("AirDashStart", true);
                    }
                    AudioSettings.PlaySFX("DoubleJump");
                }

                rb.AddForce(jumpForce * Vector3.Up, ForceMode.Impulse);
                groundCheckLocked = true;
                grounded = false;

                Console.WriteLine("Jumps left " + (maxJumps - jumpCounter));
            }
        }
        private bool CanJump()
        {
            return grounded || jumpCounter < maxJumps;
        }
        private void GroundCheck()
        {
            if (groundCheckLocked)
            {
                groundCheckTimer += Time.deltaTime;
                if (groundCheckTimer >= groundCheckDelay)
                {
                    groundCheckLocked = false;
                    groundCheckTimer = 0f;
                }
            }

            // Can only be grounded if initial delay is over
            if (!groundCheckLocked)
            {
                grounded = groundCheck.Grounded;
                if (grounded)
                {
                    jumpCounter = 0;
                    if (!isAttacking &&
                        (String.Compare(animator.GetCurrAnimName(), "Walk") == 0 && input == Vector3.Zero) ||
                    String.Compare(animator.GetCurrAnimName(), "Attack1") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "Attack2") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                        String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
                        String.Compare(animator.GetCurrAnimName(), "PlungeToIdle") == 0)
                        animator.SetBool("Idle", true);
                }
            }
        }
        private IEnumerator Dash(Vector3 dashDir)
        {
            isDashing = true;

            AudioSettings.PlaySFX("Dash");

            Vector3 dash = Vector3.Zero;

            animator.SetBool("AirDashStart", true);

            if (input == Vector3.Zero) dash = playerModel.GetComponent<Transform>().RotationQuat * transform.Backward * dashMultiplier;
            else dash = dashDir * dashMultiplier;

            rb.Velocity = new Vector3(dash.x, 0f, dash.z);

            yield return new WaitForSeconds(dashDuration);

            isDashing = false;
        }
        private void ExampleMovement()
        {
            //Vector3 right = Vector3.Cross(up, direction).Normalize();
            //float rotationSpeedFrame = rotationSpeed * Time.deltaTime;

            //if (testingShit == false)
            //{
            //    CloneGO("GameObject_2");
            //    testingShit = true;
            //}
            //// Forwards
            //if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            //{
            //    //t.Position += direction * moveSpeed * dt;

            //    transform.Position += direction * moveSpeed * Time.deltaTime;

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

            //    transform.Position -= direction * moveSpeed * Time.deltaTime;
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

            //transform.RotationQuat = Quaternion.LookRotation(this.direction, this.up);

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
            //    transform.Position += new Vector3(0, 1, 0) * moveSpeed * Time.deltaTime;
            //}


            //// Scale Down
            //if (Input.IsKeyDown(Keys.KEY_R) || Input.IsKeyDown(Keys.KEY_R))
            //{
            //    animator.SetBool("Idle", false);
            //    animator.SetBool("Run", false);
            //    animator.SetBool("Attack", true);
            //}
        }
        #endregion
        #region Attacks
        private void InitializeAttackHitboxes()
        {
            attack1HB = gameObject.FindGameObjectWithName(attack1HBName)?.As<Hitbox>();
            attack1HB.OnAttack += Attack1;
            if (attack1HB == null) Console.WriteLine("Attack 1 hitbox not found");
            else Console.WriteLine("Attack 1 hitbox found");

            attack2HB = gameObject.FindGameObjectWithName(attack2HBName)?.As<Hitbox>();
            attack2HB.OnAttack += Attack2;
            if (attack2HB == null) Console.WriteLine("Attack 2 hitbox not found");
            else Console.WriteLine("Attack 2 hitbox found");

            attack3HB = gameObject.FindGameObjectWithName(attack3HBName)?.As<Hitbox>();
            attack3HB.OnAttack += Attack3;
            if (attack3HB == null) Console.WriteLine("Attack 3 hitbox not found");
            else Console.WriteLine("Attack 3 hitbox found");
        }
        private void TryAttack()
        {
            if (!isAttacking)
            {
                attackCounter++;
                if (attackCounter > 3) attackCounter = 1;
                switch (attackCounter)
                {
                    case 1:
                        attack1HB.OnAttack.Invoke();
                        break;
                    case 2:
                        attack2HB.OnAttack.Invoke();
                        break;
                    case 3:
                        attack3HB.OnAttack.Invoke();
                        break;
                    default:
                        break;
                }
                attackResetTimer = 0f;
                Console.WriteLine("Attack Counter: " + attackCounter);
            }
        }
        private void AttackResetTimer()
        {
            if (!isAttacking) attackResetTimer += Time.deltaTime;
            if (attackResetTimer >= attackResetTime)
            {
                attackCounter = 0;
                attackResetTimer = 0f;

                if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                {
                    animator.SetBool("AttackToIdle1", true);
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                {
                    animator.SetBool("AttackToIdle2", true);
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack3") == 0)
                {
                    animator.SetBool("Attack3ToLoco", true);
                }
            }
        }
        private void Attack1()
        {
            StartCoroutine(InAttackCoroutine(attack1Duration, null));
            animator.SetBool("Attack1", true);
            AudioSettings.PlaySFX("A1");
            //if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 || String.Compare(animator.GetCurrAnimName(), "Walk") == 0)
            //{
            //    animator.SetBool("Attack1", true);
            //}
            //StartCoroutine(InAttackCoroutine(attack1Duration, () =>
            //{
            //    animator.SetBool("AttackToIdle1", true);
            //    Console.WriteLine("Attack 1 finished");
            //    Console.WriteLine(animator.GetCurrAnimName());
            //    StartCoroutine(AnimationCoroutine(1.316f, () =>
            //    {
            //        Console.WriteLine(animator.GetCurrAnimName());
            //        animator.SetBool("Idle", true);
            //        Console.WriteLine("Returned to idle");
            //    }));
            //}));
            List<EnemySlime> enemiesHit = attack1HB.EnemiesInRange;
            foreach (EnemySlime enemy in enemiesHit)
            {
                enemy.TakeDamage(attack1Damage);
            }
            Console.WriteLine("Attack 1 executed");
        }
        private void Attack2()
        {
            if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
            {
                animator.SetBool("Attack2", true);
            }
            StartCoroutine(InAttackCoroutine(attack2Duration, null));
            AudioSettings.PlaySFX("A2");
            //animator.SetBool("Attack2", true);
            //StartCoroutine(InAttackCoroutine(attack2Duration, () =>
            //{
            //    animator.SetBool("AttackToIdle2", true);
            //    Console.WriteLine("Attack 2 finished");
            //    StartCoroutine(AnimationCoroutine(0.816f, () =>
            //    {
            //        animator.SetBool("Idle", true);
            //        Console.WriteLine("Returned to idle");
            //    }));
            //}));
            List<EnemySlime> enemiesHit = attack2HB.EnemiesInRange;
            foreach (EnemySlime enemy in enemiesHit)
            {
                enemy.TakeDamage(attack2Damage);
            }
            Console.WriteLine("Attack 2 executed");
        }
        private void Attack3()
        {
            if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
            {
                animator.SetBool("Attack3", true);
            }
            StartCoroutine(InAttackCoroutine(attack3Duration, null));
            AudioSettings.PlaySFX("A3");
            //animator.SetBool("Attack3", true);
            //StartCoroutine(InAttackCoroutine(attack3Duration, null));
            List<EnemySlime> enemiesHit = attack3HB.EnemiesInRange;
            foreach (EnemySlime enemy in enemiesHit)
            {
                enemy.TakeDamage(attack3Damage);
            }
            Console.WriteLine("Attack 3 executed");
        }
        private IEnumerator InAttackCoroutine(float duration, Action endAction)
        {
            canMove = false;
            isAttacking = true;
            Console.WriteLine("Is attacking");
            yield return new WaitForSeconds(duration);
            Console.WriteLine("Can attack");
            endAction?.Invoke();
            canMove = true;
            isAttacking = false;
        }
        private bool AttackAnimationState()
        {
            if (!isAttacking)
            {
                if (String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "Attack1") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "Attack2") == 0 ||
                    String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 || 
                    String.Compare(animator.GetCurrAnimName(), "Attack3") == 0)
                    return true;
            }
            return false;
        }
        private void AnimationStateHandling()
        {
            if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
            {
                animator.SetBool("AttackToIdle1", true);
            }
            if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
            {
                animator.SetBool("AttackToIdle2", true);
            }
            if (String.Compare(animator.GetCurrAnimName(), "Attack3") == 0)
            {
                animator.SetBool("Attack3ToLoco", true);
            }

            if (isAttacking)
            {
                if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                {
                    animator.SetBool("Attack2", false);
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                {
                    animator.SetBool("Attack3", false);
                }
            }
            if (!isAttacking)
            {
                if (String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
                String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
                String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0)
                    animator.SetBool("Idle", true);
            }
        }
        private IEnumerator AnimationCoroutine(float duration, Action endAction)
        {
            yield return new WaitForSeconds(duration);
            endAction?.Invoke();
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

        #region On Overrides
        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source) 
        {
            Console.WriteLine("Player Taking Damage. Current Health: ");
            Console.WriteLine(currentHealth);
            Bootstrap.HUDManager.SetHealth((float)currentHealth / (float)maxHealth);
        }

        private bool isDead = false;

        public override void OnDeath()
        {
            if (!isDead)
            {
                isDead = true;

                Bootstrap.LevelDirector.Lose();
                //this.gameObject.Destroy();
            }
        }
        #endregion
    }
}

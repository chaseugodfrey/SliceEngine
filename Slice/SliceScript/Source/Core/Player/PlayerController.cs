using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;


namespace SliceEngine
{

    public class PlayerController : Entity, IInitializable
    {
        public float moveSpeed = 2.5f;
        public float rotSpeed = 12f;
        public bool isGrounded = false;
        public bool isJumping = false;
        public bool isDoubleJumping = false;
        public bool isGroundDashing = false;
        public bool isAirDashing = false;
        public bool canInput = true;
        public bool canMove = true;
        public bool iFrames = false;
        public GameObject playerModel;
        public GameObject cameraObject;
        public float dashDuration = 0.75f;
        public float dashCooldown = 0.6f;
        public float dashSpeed = 10.0f;
        public float jumpSpeed = 15.0f;
        public float fallSpeed = 25.0f;
        public float fallTransitionTime = 0.25f;
        public float lungeDuration = 0.5f;
        public float lungeSpeed = 5.0f;
        public float iFrameDuration = 0.2f;
        public float flickerDuration = 0.05f;

        //public bool attackAutoRecover = false;
        private float dashCooldownTimer = 0.0f;
        private float dashTimer = 0.0f;
        private float fallTimer = 0.0f;
        private Vector3 finalMove;
        private Vector3 input;
        private Vector3 dashInputDir;
        private RigidBody rb;
        private Animator animator;
        private CameraController camera;
        private int groundTriggerCount = 0;
        private string groundName = "Ground";
        private bool jumpRequest = false;
        private bool dashRequest = false;

        // Attacking Variables
        public float attackResetTime = 1f;
        private float attackResetTimer = 0f;
        public bool isAttacking = false;
        private int attackCounter = 0;
        public float attackRecoveryDuration = 0.5f;
        private bool attackQueued = false;
        private bool attackAutoRecover = false;
        public float attack1Delay, attack2Delay, attack3Delay;

        // no plunging for now
        private bool isPlunging = false;

        public string attack1HBName;
        public int attack1Damage;
        private GeneralHitbox attack1HB;

        public string attack2HBName;
        public int attack2Damage;
        private GeneralHitbox attack2HB;

        public string attack3HBName;
        public int attack3Damage;
        private GeneralHitbox attack3HB;

        private int attackIndex = 0;
        private float attackTimer = 0f;
        private bool queuedNext = false;

        private bool isLunging = false;
        private float lungeTimer = 0.0f;
        private bool canIncrement = true;
        private bool isDead = false;

        // Just to debug shit
        int count = 0;

        #region Entity Overrides
        public void Initialize()
        {
            Bootstrap.HUDManager.SetHealth(currentHealth / maxHealth);
        }
        public override void OnDeath()
        {
            if (!isDead)
            {
                isDead = true;

                Bootstrap.LevelDirector.Lose();
                //this.gameObject.Destroy();
            }

        }
        protected override void OnHeal()
        {

        }
        protected override void OnDamaged(GameObject source)
        {
            Bootstrap.HUDManager.SetHealth((float)currentHealth / (float)maxHealth);

            if (!iFrames)
            {
                iFrames = true;
                StartCoroutine(iFrameAnimation(iFrameDuration));
            }
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            // If its iFrames, dont take damage
            if (iFrames)
                return;
            base.TakeDamage(amount, source);
        }
        #endregion

        public override void OnAwake()
        {

        }

        public override void OnCreate()
        {
            animator = playerModel?.GetComponent<Animator>();
            rb = GetComponent<RigidBody>();
            camera = cameraObject.As<CameraController>();

            // InitializeAttackHitboxes();
        }

        public override void OnUpdate(float dt)
        {
            if (dashCooldownTimer > 0.0f) dashCooldownTimer -= dt;

            if (canInput)
            {
                HandleInput();

                // Capture jump input in OnUpdate
                if (Input.IsKeyPressed(Keys.KEY_SPACEBAR))
                {
                    if (isGrounded && !isAttacking) jumpRequest = true;
                    else if (isJumping && !isDoubleJumping) jumpRequest = true;
                }
            }

            if (isGroundDashing || isAirDashing)
            {
                dashTimer -= dt;
                if (dashTimer <= 0.0f)
                {
                    dashCooldownTimer = dashCooldown;
                    isGroundDashing = false;
                    isAirDashing = false;

                    // Clear Dash bools when finished
                    animator?.SetBool("BackDashStart", false);
                    animator?.SetBool("DashStart", false);
                    Console.WriteLine("Dash Ending");
                }
            }

            if (isLunging)
            {
                lungeTimer -= dt;
                if (lungeTimer <= 0.0f)
                {
                    isLunging = false;
                }
            }

            if (attackQueued && !isAttacking)
            {
                ExecuteAttack();
                attackQueued = false;
            }

            UpdateRotation(dt);
            UpdateAnimation(); // Centralized animation control
            AttackResetTimer();
        }

        public override void OnFixedUpdate(float dt)
        {
            ApplyDash();
            HandleMovement(dt);
            ApplyJump();
        }

        void HandleInput()
        {
            input = Vector3.Zero;
            if (canInput)
            {
                if (Input.IsKeyDown(Keys.KEY_W)) input += new Vector3(0f, 0f, 1f);
                else if (Input.IsKeyDown(Keys.KEY_S)) input += new Vector3(0f, 0f, -1f);

                if (Input.IsKeyDown(Keys.KEY_A)) input += new Vector3(1f, 0f, 0f);
                else if (Input.IsKeyDown(Keys.KEY_D)) input += new Vector3(-1f, 0f, -0f);

                if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_RIGHT))
                {
                    dashRequest = true;
                }
            }

            if (input.SquareMagnitude() > 1f) input = input.Normalize();

            if (canInput)
            {
                if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
            }
        }

        void UpdateAnimation()
        {
            if (animator == null) return;

            bool isDashing = isGroundDashing || isAirDashing;

            if (isDashing)
            {
                animator.SetBool("Walk", false);
                animator.SetBool("Idle", false);
                animator.SetBool("JumpLoop", false);
                animator.SetBool("Fall", false);
                return;
            }

            if (!isGrounded)
            {
                animator.SetBool("Walk", false);
                animator.SetBool("Idle", false);

                // Transition to Falling if vertical velocity is downward
                //if (rb != null && rb.Velocity.y < -0.1f)
                if (fallTimer > fallTransitionTime)
                {
                    //Console.WriteLine($"Transitioning to fall {count++}");
                    if (animator.SafeToChange("Fall"))
                        animator.SetBool("Fall", true);

                    animator.SetBool("JumpLoop", false);
                }
                return;
            }


            if (input.SquareMagnitude() > 0.01f)
            {
                if (animator.SafeToChange("Walk"))
                    animator.SetBool("Walk", true);
                animator.SetBool("Idle", false);
            }
            else
            {
                if (animator.SafeToChange("Idle"))
                    animator.SetBool("Idle", true);
                animator.SetBool("Walk", false);
            }

            // Cleanup air flags when on ground
            animator.SetBool("JumpLoop", false);
            animator.SetBool("AirDashStart", false);
            animator.SetBool("Fall", false);
        }

        #region Movement
        void UpdateRotation(float dt)
        {
            if (isGroundDashing || isAirDashing || !canMove || isAttacking || isLunging || attackAutoRecover || isPlunging) return;

            // Safer than zero vector check since floating point error sometimes
            if (input.SquareMagnitude() > 0.0001f)
            {
                Vector3 camForward = Vector3.Zero;
                if (camera != null)
                {
                    camForward = camera.transform.RotationQuat * Vector3.Forward;
                    camForward.y = 0f;
                    camForward = camForward.Normalize();
                }

                Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
                Vector3 moveDirInput = camForward * input.z + camRight * input.x;

                if (moveDirInput.SquareMagnitude() > 0.0001f)
                {
                    Quaternion targetRot = Quaternion.LookRotation(moveDirInput, Vector3.Up);
                    transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRot, rotSpeed * dt);
                }
            }
        }

        void HandleMovement(float dt)
        {
            Vector3 camForward = Vector3.Zero;
            if (camera != null)
            {
                camForward = camera.transform.RotationQuat * Vector3.Forward;
                camForward.y = 0f;
                camForward = camForward.Normalize();
            }

            Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
            Vector3 moveDirInput = camForward * input.z + camRight * input.x;

            if (!isGrounded && rb != null && rb.Velocity.y < -0.1f)
            {
                fallTimer += dt;
                Console.WriteLine($"fall timer: {fallTimer}");
            }
            // If its not falling then reset fall timer
            else
            {
                fallTimer = 0.0f;
            }

            if (isLunging)
            {
                Vector3 lungeDir = transform.Forward;
                lungeDir.y = 0f;
                lungeDir = lungeDir.Normalize();
                Vector3 lungeVel = lungeDir * lungeSpeed;
                rb.Velocity = new Vector3(lungeVel.x, rb.Velocity.y, lungeVel.z);
            }
            else if (isGroundDashing || isAirDashing)
            {
                Vector3 dashVel = dashInputDir * dashSpeed;
                float yVel = isGroundDashing ? 0 : rb.Velocity.y;
                rb.Velocity = new Vector3(dashVel.x, yVel, dashVel.z);
            }
            else
            {
                if (!isGrounded)
                {
                    Vector3 vel = rb.Velocity;
                    vel.y -= fallSpeed * dt;
                    rb.Velocity = vel;
                }

                if (canMove && !isAttacking && !attackAutoRecover)
                {
                    Vector3 horizontal = moveDirInput * movementSpeed;
                    rb.Velocity = new Vector3(horizontal.x, rb.Velocity.y, horizontal.z);
                }
                else
                {
                    rb.Velocity = new Vector3(0, rb.Velocity.y, 0);
                }
            }
        }

        void ApplyDash()
        {
            if (dashRequest)
            {
                dashRequest = false;
                if (dashCooldownTimer > 0.0f) return;

                animator?.SetBool("Walk", false);
                animator?.SetBool("Idle", false);

                if (isGrounded)
                {
                    Vector3 backDir = -transform.Forward;
                    backDir.y = 0f;
                    dashInputDir = backDir.Normalize();

                    isGroundDashing = true;
                    dashTimer = dashDuration;

                    if (animator != null && animator.SafeToChange("BackDashStart"))
                        animator.SetBool("BackDashStart", true);
                }
                else
                {
                    dashInputDir = ComputeFlatDashDir(true);

                    if (dashInputDir.SquareMagnitude() > 0.0001f)
                    {
                        transform.RotationQuat = Quaternion.LookRotation(dashInputDir, Vector3.Up);
                    }

                    isAirDashing = true;
                    dashTimer = dashDuration;

                    if (animator != null && animator.SafeToChange("AirDashStart"))
                        animator.SetBool("AirDashStart", true);
                }
            }
        }

        void ApplyJump()
        {
            if (jumpRequest)
            {
                jumpRequest = false;

                animator?.SetBool("Walk", false);
                animator?.SetBool("Idle", false);

                if (isGrounded)
                {
                    isJumping = true;
                    rb.Velocity = new Vector3(rb.Velocity.x, jumpSpeed, rb.Velocity.z);
                    animator?.SetBool("JumpLoop", true);
                }
                else if (isJumping && !isDoubleJumping)
                {
                    isDoubleJumping = true;
                    rb.Velocity = new Vector3(rb.Velocity.x, jumpSpeed, rb.Velocity.z);
                    animator?.SetBool("AirDashStart", true);
                }
            }
        }

        Vector3 ComputeFlatDashDir(bool useMoveDir)
        {
            bool hasInput = input.SquareMagnitude() > 0.0001f;
            Vector3 normInput = hasInput ? input.Normalize() : Vector3.Zero;

            if (!useMoveDir || !hasInput)
            {
                Vector3 forward = transform.Forward;
                forward.y = 0f;
                return forward.Normalize();
            }

            if (camera != null)
            {
                Vector3 camForward = camera.transform.RotationQuat * Vector3.Forward;
                camForward.y = 0f;
                camForward = camForward.Normalize();
                Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
                Vector3 dashDir = camForward * normInput.z + camRight * normInput.x;
                dashDir.y = 0f;
                return dashDir.Normalize();
            }

            Vector3 moveDir = transform.Forward * normInput.z + transform.Right * normInput.x;
            moveDir.y = 0f;
            return moveDir.Normalize();
        }

        #endregion
        #region Attacking 
        // attacking should be hte same as last time I dont think I have to redo anything 
        // only the lunge

        private void InitializeAttackHitboxes()
        {
            attack1HB = gameObject.FindGameObjectWithName(attack1HBName)?.As<GeneralHitbox>();
            attack1HB.HitBoxListeners += Attack1;
            //if (attack1HB == null) Console.WriteLine("Attack 1 hitbox not found");
            //else Console.WriteLine("Attack 1 hitbox found");

            attack2HB = gameObject.FindGameObjectWithName(attack2HBName)?.As<GeneralHitbox>();
            attack2HB.HitBoxListeners += Attack2;
            //if (attack2HB == null) Console.WriteLine("Attack 2 hitbox not found");
            //else Console.WriteLine("Attack 2 hitbox found");

            attack3HB = gameObject.FindGameObjectWithName(attack3HBName)?.As<GeneralHitbox>();
            attack3HB.HitBoxListeners += Attack3;
            //if (attack3HB == null) Console.WriteLine("Attack 3 hitbox not found");
            //else Console.WriteLine("Attack 3 hitbox found");

            if (attack1HB != null && attack2HB != null && attack3HB != null)
            {
                //Console.WriteLine("All attack hitboxes found, turning them off");
                TurnOffHitboxes();
            }
        }
        private void TryAttack()
        {
            // transition to plunge if in air
            //if (!grounded && isPlunging == false)
            //{
            //    StartCoroutine(Plunge(plungeDuration));
            //    return;
            //}
            attackQueued = true;
            //console.writeline("AttackQueued set to true");
        }
        private void ExecuteAttack()
        {
            if (!isAttacking && !isPlunging)
            {
                attackAutoRecover = false;
                attackCounter++;
                isAttacking = true;

                if (attackCounter > 3) attackCounter = 1;
                switch (attackCounter)
                {
                    case 1:
                        //StartCoroutine(AttackDelay(attack1Delay, () => attack1HB.TurnOn()));

                        animator.SetBool("Attack1", true);
                        AudioSettings.PlaySFX("A1");

                        isLunging = true;
                        lungeTimer = lungeDuration;
                        break;
                    case 2:
                        //StartCoroutine(AttackDelay(attack1Delay, () => attack2HB.TurnOn()));

                        if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                        {
                            animator.SetBool("Attack2", true);
                            AudioSettings.PlaySFX("A2");
                        }

                        isLunging = true;
                        lungeTimer = lungeDuration;
                        break;
                    case 3:
                        //StartCoroutine(AttackDelay(attack1Delay, () => attack3HB.TurnOn()));

                        if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                        {
                            animator.SetBool("Attack3", true);
                            AudioSettings.PlaySFX("A3");
                        }
                        break;
                    default:
                        break;
                }
                //console.writeline("Attack Counter: " + attackCounter);
            }
        }
        private void TurnOffHitboxes()
        {
            //attack1HB.TurnOff();
            //attack2HB.TurnOff();
            //attack3HB.TurnOff();
        }
        public void StartAttackRecovery()
        {
            attackResetTimer = 0f;
            isAttacking = false;
            attackAutoRecover = true;

            TurnOffHitboxes();
        }
        private void AttackResetTimer()
        {
            if (!isAttacking)
            {
                attackResetTimer += Time.deltaTime;
            }
            if (attackResetTimer >= attackRecoveryDuration)
            {
                attackAutoRecover = false;
                attackResetTimer = 0f;
                attackCounter = 0;

                if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                {
                    if (animator.SafeToChange("AttackToIdle1"))
                        animator.SetBool("AttackToIdle1", true);
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                {
                    if (animator.SafeToChange("AttackToIdle2"))
                        animator.SetBool("AttackToIdle2", true);
                }
                if (String.Compare(animator.GetCurrAnimName(), "Attack3") == 0)
                {
                    if (animator.SafeToChange("Attack3ToLoco"))
                        animator.SetBool("Attack3ToLoco", true);
                }
            }
        }
        private IEnumerator Plunge(float duration)
        {
            //console.writeline("Plunging");
            //isPlunging = true;
            //if (animator != null)
            //{
            //    animator.SetBool("Plunge", true);
            //}
            float timer = 0f;
            while (timer < duration)
            {
                Vector3 velTemp = rb.Velocity;
                velTemp.y = 0.0f;
                rb.Velocity = velTemp;
                //velocity.y = 0f;
                timer += Time.deltaTime;
                yield return null;
            }
        }

        private IEnumerator iFrameAnimation(float duration)
        {
            float timer = 0.0f;
            float flickerTimer = 0.0f;
            while (timer < duration)
            {
               //Vector4 col = playerModel.GetComponent<Renderer>().GetColor();
                
                if (flickerTimer >= flickerDuration)
                {
                    playerModel.As<PlayerAnimatorEvents>().FlickerModel();

                    //if (col.z == 0.0f)
                    //{
                    //    col.z = 1.0f;

                    //}
                    //else if (col.z == 1.0f)
                    //{
                    //    col.z = 0.0f;
                    //}

                    flickerTimer = 0.0f;
                }

               // playerModel.GetComponent<Renderer>().SetColor(col);


                timer += Time.deltaTime;
                flickerTimer += Time.deltaTime;
                yield return null;
            }

            iFrames = false;
        }

        private void Attack1(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attack1Damage);
                //console.writeline("Hit enemy");
            }
            //console.writeline("Attack 1 executed");
        }
        private void Attack2(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attack2Damage);
                //console.writeline("Hit enemy");
            }
            //console.writeline("Attack 2 executed");
        }
        private void Attack3(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attack3Damage);
                //console.writeline("Hit enemy");
            }
            //console.writeline("Attack 3 executed");
        }
        private IEnumerator AttackDelay(float delay, Action action)
        {
            yield return new WaitForSeconds(delay);
            action.Invoke();
        }
        public void CanAttackFlag(bool flag)
        {
            canIncrement = flag;
        }
        public void CanMoveFlag(bool flag)
        {
            canMove = flag;
        }
        void EndAttackState()
        {
            attackIndex = 0;
            attackTimer = 0f;
            attackCounter = 0;
            attackQueued = false;
            isAttacking = false;
            attackResetTimer = 0f;
            TurnOffHitboxes();

            queuedNext = false;

            // Clear movement bursts
            isLunging = false;
        }
        #endregion

        public void TeleportPlayer(Vector3 pos)
        {
            EndAttackState();
            transform.Position = pos;
        }

        public override void OnCollideEnter(uint other)
        {
            if (gameObject.FindGameObjectWithID(other).tag == groundName)
            {
                isGrounded = true;
                groundTriggerCount += 1;

                if (isJumping || isDoubleJumping)
                    animator?.SetBool("Land", true);

                if (isJumping) isJumping = false;
                if (isDoubleJumping) isDoubleJumping = false;
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (gameObject.FindGameObjectWithID(other).tag == groundName)
            {
                groundTriggerCount -= 1;
                groundTriggerCount = Utilities.Clamp<int>(groundTriggerCount, 0, 999);
                if (groundTriggerCount <= 0)
                {
                    isGrounded = false;
                }
            }
        }
    }
}
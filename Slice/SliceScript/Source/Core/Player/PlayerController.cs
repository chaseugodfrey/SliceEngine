using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using static SliceEngine.GeneralHitbox;


namespace SliceEngine
{

    public class PlayerController : Entity, IInitializable
    {
        // Debug Mode
        public bool debugMode = false;

        public enum MovementState
        {
            Idle,
            Walking,
            Jumping,
            DoubleJumping,
            Falling,
            Landing,
            GroundDash,
            AirDash,
            Lunging,
            Plunging,
            Dead
        }

        public enum CombatState
        {
            None,
            Attacking,
            Recovery,
            Hitstun
        }

        public enum CurrentAttack
        {            
            Attack1,
            Attack2,
            Attack3,
            PlungeLand,
            None
        }

        public enum ControlState
        {
            Gameplay,
            Disabled,
            Teleporting,
            Cutscene
        }

        public MovementState playerMovementState = MovementState.Idle;
        public CombatState playerCombatState = CombatState.None;
        public CurrentAttack playerCurrentAttack = CurrentAttack.None;
        public ControlState playerControlState = ControlState.Gameplay;

        // Internal References
        private CameraController camera;
        public GameObject playerModel;
        RigidBody rigidBody;
        Animator animator;
        AudioSource audio;

        // Attacks
        List<GeneralHitbox> attackHitboxes = new List<GeneralHitbox>();
        public List<String> attackHitboxNames = new List<String>();
        public List<int> attackDamageValues = new List<int>();
        public List<float> attackDuration = new List<float>();
        public List<Vector3> attackWindows = new List<Vector3>();
        float attackTimer = 0.0f;
        bool attackQueued;
        Coroutine attackCoroutine;

        // Lunging (moving when attacking)
        bool isLunging = false;
        float lungeTimer = 0f;
        public float lungeDuration = 0f;
        float lungeDelay = 0f;
        Vector3 lungeDir = Vector3.Zero;
        public float lungeSpeed = 0f;

        // Attack 3 Arc runtime
        bool atk3ArcActive = false;
        float atk3ArcTimer = 0f;
        bool atk3ImpulseFired = false;
        Vector3 atk3ArcDir = Vector3.Zero;
        Vector3 atk3HorizVel = Vector3.Zero;
        float atk3UpwardEndTime = 0f;

        // Ground Check
        public float groundCheckDelay = 0.1f;
        float groundCheckTimer = 0f;
        public bool grounded;
        private bool groundedTemp;
        public bool groundCheckLocked;
        public int groundContactCount;

        // Jumps
        int jumpCounter = 0;
        public int jumpMax = 2;
        public float jumpVerticalForce = 10.0f;
        public float jumpHorizontalForce = 7.5f;
        public float jumpDuration = 0.25f;
        public float jumpCooldown = 0.25f;
        public float jumpLandDuration = 0.25f;
        float jumpDurationTimer = 0.0f;
        float jumpCooldownTimer = 0.0f;
        float jumpLandTimer = 0.0f;

        // Movement
        private Vector3 input;
        private Vector3 finalMove;
        private Vector3 velocity;
        public float moveSpeed = 2.5f;
        public float moveAcceleration = 100.0f;
        public float moveDeceleration = 100.0f;
        public float rotationSpeed = 45.0f;

        // Dash
        public float dashDuration = 0.25f;
        public float dashCooldown = 0.75f;
        float dashDurationTimer = 0.0f;
        float dashCooldownTimer = 0.0f;

        float allowedDashDistance;
        public float dashDistance = 6f;
        public float dashStartDuration = 0.25f;
        bool rotateToDashDirection = false;
        bool dashDefaultBackwards = true;
        public bool dashUsesMoveDirection = true;
        Vector3 dashDir = Vector3.Zero;
        public void Initialize()
        {
            camera = Bootstrap.CameraController; if (camera == null) SliceLog.Warn("PlayerController cannot find camera");

            Bootstrap.HUDManager.SetHealth(currentHealth / maxHealth);
        }

        public override void OnCreate()
        {            
            InitializeInternalReferences();
            //InitializeAttacks();
        }

        public override void OnUpdate(float dt)
        {
            MovementState remember = playerMovementState;

            GroundCheck();
            HandleInputs();

            UpdateTimers(dt);
            UpdateMovements(dt);
            UpdateAttacks(dt);
            UpdateStates();
            UpdateAnimator();

            if (remember != playerMovementState)
            {
                SliceLog.Log(playerMovementState.ToString());
            }
        }
        private void HandleInputs()
        {
            input = Vector3.Zero;

            HandleMovementInputs();

            HandleAttackInputs();

            HandleDashInputs();

            HandleJumpInputs();
        }

        //void EndAttackState()
        //{
        //    attackIndex = 0;
        //    attackTimer = 0f;
        //    attackCounter = 0;
        //    attackQueued = false;
        //    playerCombatState = CombatState.None;
        //    attackResetTimer = 0f;
        //    TurnOffHitboxes();

        //    queuedNext = false;
        //    queuedFacingOverride = false;

        //    // Clear movement bursts
        //    isLunging = false;
        //    lungeStarted = false;

        //    atk3ArcActive = false;
        //    atk3ImpulseFired = false;
        //    atk3HorizVel = Vector3.Zero;
        //}
        
        //private void ExecuteAttack()
        //{
        //    if (playerCombatState != CombatState.Attacking && playerMovementState != MovementState.Plunge)
        //    {
        //        attackCounter++;
        //        playerCombatState = CombatState.Attacking;

        //        if (attackCounter > 3) attackCounter = 1;
        //        switch (attackCounter)
        //        {
        //            case 1:
        //                StartCoroutine(AttackDelay(attack1Delay, () => attack1HB.TurnOn()));

        //                animator.SetBool("Attack1", true);
        //                AudioSettings.PlaySFX("A1");

        //                StartCoroutine(Lunge());
        //                break;
        //            case 2:
        //                StartCoroutine(AttackDelay(attack1Delay, () => attack2HB.TurnOn()));

        //                if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
        //                {
        //                    animator.SetBool("Attack2", true);
        //                    AudioSettings.PlaySFX("A2");
        //                }

        //                StartCoroutine(Lunge());
        //                break;
        //            case 3:
        //                StartCoroutine(AttackDelay(attack1Delay, () => attack3HB.TurnOn()));

        //                if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
        //                {
        //                    animator.SetBool("Attack3", true);
        //                    AudioSettings.PlaySFX("A3");
        //                }
        //                break;
        //            default:
        //                break;
        //        }
        //        //console.writeline("Attack Counter: " + attackCounter);
        //    }
        //}
        //private IEnumerator Lunge()
        //{
        //    float timer = 0f;
        //    while (timer < lungeDuration)
        //    {
        //        transform.Position += transform.Forward * lungeSpeed * Time.deltaTime;
        //        timer += Time.deltaTime;
        //        yield return null;
        //    }
        //}
        //private void TurnOffHitboxes()
        //{
        //    foreach (GeneralHitbox hb in attackHitboxes)
        //    {
        //        hb.TurnOff();
        //    }
        //}
        //public void StartAttackRecovery()
        //{
        //    attackResetTimer = 0f;
        //    playerCombatState = CombatState.Recovery;
        //    attackAutoRecover = true;

        //    TurnOffHitboxes();
        //}
        //private void AttackResetTimer()
        //{
        //    if (playerCombatState != CombatState.Attacking)
        //    {
        //        attackResetTimer += Time.deltaTime;
        //    }
        //    if (attackResetTimer >= attackRecoveryDuration)
        //    {
        //        attackAutoRecover = false;
        //        attackResetTimer = 0f;
        //        attackCounter = 0;

        //        if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
        //        {
        //            if (animator.SafeToChange("AttackToIdle1"))
        //                animator.SetBool("AttackToIdle1", true);
        //        }
        //        if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
        //        {
        //            if (animator.SafeToChange("AttackToIdle2"))
        //                animator.SetBool("AttackToIdle2", true);
        //        }
        //        if (String.Compare(animator.GetCurrAnimName(), "Attack3") == 0)
        //        {
        //            if (animator.SafeToChange("Attack3ToLoco"))
        //                animator.SetBool("Attack3ToLoco", true);
        //        }
        //    }
        //}

        private void Attack1(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attackDamageValues[(int)CurrentAttack.Attack1]);
            }
        }
        private void Attack2(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attackDamageValues[(int)CurrentAttack.Attack2]);
            }
        }
        private void Attack3(GameObject target){
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attackDamageValues[(int)CurrentAttack.Attack3]);
            }
        }
        private IEnumerator AttackDelay(float delay, Action action)
        {
            yield return new WaitForSeconds(delay);
            action.Invoke();
        }
        private bool AttackAnimationState()
        {
            if (playerCombatState != CombatState.Attacking)
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

            if (playerCombatState == CombatState.Attacking)
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
            if (playerCombatState != CombatState.Attacking)
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

        #region On Overrides
        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source)
        {
            //console.writeline("Player Taking Damage. Current Health: ");
            //console.writeline(currentHealth);
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

        #region Helpers
        private void UpdateTimers(float dt)
        {
            lungeTimer = (lungeTimer > 0.0f) ? lungeTimer - dt : 0.0f;
            jumpCooldownTimer = (jumpCooldownTimer > 0.0f) ? jumpCooldownTimer - dt : 0.0f;
            dashCooldownTimer = (dashCooldownTimer > 0.0f) ? dashCooldownTimer - dt : 0.0f;
            dashDurationTimer = (dashDurationTimer > 0.0f) ? dashDurationTimer - dt : 0.0f;
            attackTimer = (attackTimer > 0.0f) ? attackTimer - dt : 0.0f;
            jumpDurationTimer = (jumpDurationTimer > 0.0f) ? jumpDurationTimer - dt : 0.0f;
            jumpLandTimer = (jumpLandTimer > 0.0f) ? jumpLandTimer - dt : 0.0f;
        }
        private void UpdateMovements(float dt)
        {
            Vector3 camForward = new Vector3();
            finalMove = Vector3.Zero;
            if (camera != null)
            {
                camForward = camera.transform.RotationQuat * Vector3.Forward; // Get camera forward direction
                camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
                camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera
            }

            Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
            Vector3 moveDirInput = camForward * input.z + camRight * input.x;
            float rawPlanarSpeed = moveDirInput.Magnitude() * movementSpeed;

            if (playerMovementState == MovementState.GroundDash || playerMovementState == MovementState.AirDash)
            {
                Dash(dashDir, dt);
            }
            else if (playerMovementState == MovementState.Jumping || playerMovementState == MovementState.Falling)
            {
                // Let pure physics do this part
            }
            if (playerMovementState == MovementState.Idle || playerMovementState == MovementState.Walking)
            {
                // Normal locomotion
                if (moveDirInput.SquareMagnitude() > 0.0001f)
                {
                    Quaternion targetRot = Quaternion.LookRotation(moveDirInput, Vector3.Up);
                    float scaledRotSpeed = rotationSpeed;
                    transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRot, scaledRotSpeed * Time.deltaTime);
                }

                finalMove = moveDirInput * movementSpeed;
                transform.Position += finalMove * dt;
            }
        }

        void UpdateAttacks(float dt)
        {
            if (attackQueued)
            {
                if (!grounded && playerMovementState != MovementState.Plunging)
                {
                    playerMovementState = MovementState.Plunging;
                    attackQueued = false;
                }

                if (grounded)
                {
                    playerMovementState = MovementState.Idle;
                    playerCombatState = CombatState.Attacking;
                    //ExecuteAttack();
                    attackQueued = false;
                }
            }
        }

        void UpdateStates()
        {            
            switch (playerMovementState)
            {
                case MovementState.Idle:
                    if (!grounded && jumpDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Falling;
                    }
                    else if (input != Vector3.Zero)
                    {
                        playerMovementState = MovementState.Walking;
                    }
                    
                    break;
                case MovementState.Walking:
                    if (!grounded && jumpDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Falling;
                    }
                    else if (input == Vector3.Zero)
                    {
                        playerMovementState = MovementState.Idle;
                    }
                    break;
                case MovementState.Jumping:
                    if (!grounded && jumpDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Falling;
                    }
                    else if (grounded && jumpDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Idle;
                    }
                    break;

                case MovementState.DoubleJumping:
                    if (!grounded && jumpDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Falling;
                    }
                    else if (grounded && jumpDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Idle;
                    }
                    break;
                case MovementState.Falling:
                    if (grounded)
                    {
                        jumpLandTimer = jumpLandDuration;
                        playerMovementState = MovementState.Landing;
                    }
                    break;
                case MovementState.Landing:
                    if (jumpLandTimer <= 0.0f)
                    {
                        playerMovementState = input == Vector3.Zero ? MovementState.Idle : MovementState.Walking;
                    }
                    break;
                case MovementState.GroundDash:
                    if (dashDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Idle;
                    }
                    break;

                case MovementState.AirDash:
                    if (dashDurationTimer <= 0.0f)
                    {
                        playerMovementState = MovementState.Falling;
                    }
                    break;

                case MovementState.Lunging:
                    if (lungeDuration == 0.0f)
                    {
                        playerMovementState = MovementState.Idle;
                    }
                    break;

                case MovementState.Plunging:
                    if (grounded)
                    {
                        playerMovementState = MovementState.Landing;
                        playerCurrentAttack = CurrentAttack.PlungeLand;
                        jumpLandTimer = jumpLandDuration;
                    }
                    break;

                case MovementState.Dead:
                    break;

                default:
                    break;
            }            

            switch (playerCombatState)
            {
                case CombatState.None:
                    break;
                case CombatState.Attacking:
                    break;
                case CombatState.Recovery:    
                    break;
                case CombatState.Hitstun:
                    break;
                default:
                    break;
            }
        }


        void UpdateAnimator()
        {
            if (animator == null)
                return;

            switch (playerMovementState)
            {
                case MovementState.Idle:
                    if (animator.SafeToChange("Idle"))
                        animator.SetBool("Idle", true);
                    break;
                case MovementState.Walking:
                    if (animator.SafeToChange("Walk"))
                        animator.SetBool("Walk", true);
                    break;
                case MovementState.Jumping:
                    if (animator.SafeToChange("JumpLoop"))
                        animator.SetBool("JumpLoop", true);
                    break;
                case MovementState.DoubleJumping:
                    if (animator.SafeToChange("BackDashStart"))
                        animator.SetBool("BackDashStart", true);
                    break;
                case MovementState.Falling:
                    if (animator.SafeToChange("Fall"))
                        animator.SetBool("Fall", true);
                    break;
                case MovementState.Landing:
                    if (animator.SafeToChange("Land"))
                        animator.SetBool("Land", true);
                    break;
                case MovementState.GroundDash:
                    animator.SetBool("DashStart", true);
                    break;

                case MovementState.AirDash:
                    animator.SetBool("AirDashStart", true);
                    break;

                case MovementState.Lunging:
                    break;

                case MovementState.Plunging:
                    if (input != Vector3.Zero)
                        animator.SetBool("PlungeToWalk", true);
                    else
                        animator.SetBool("PlungeToIdle", true);
                    break;

                case MovementState.Dead:
                    break;

                default:
                    break;
            }
            switch (playerCombatState)
            {
                case CombatState.None:
                    break;
                case CombatState.Attacking:
                    animator.SetBool(playerCurrentAttack.ToString(), true);
                    break;
                case CombatState.Recovery:
                    break;
                case CombatState.Hitstun:
                    break;
                default:
                    break;
            }
        }

        private void HandleMovementInputs()
        {
            if (IsTakingInputs())
            {
                if (Input.IsKeyDown(Keys.KEY_W)) input += new Vector3(0f, 0f, 1f);
                else if (Input.IsKeyDown(Keys.KEY_S)) input += new Vector3(0f, 0f, -1f);

                // Sideways movement 
                if (Input.IsKeyDown(Keys.KEY_A)) input += new Vector3(1f, 0f, 0f);
                else if (Input.IsKeyDown(Keys.KEY_D)) input += new Vector3(-1f, 0f, 0f);
            }

            if (input.SquareMagnitude() > 1f) input = input.Normalize();            
        }

        private void HandleAttackInputs()
        {
            if (IsTakingInputs())
            {
                if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
            }
        }

        private void HandleJumpInputs()
        {
            if (IsTakingInputs())
            {
                if (Input.IsKeyDown(Keys.KEY_SPACEBAR)) TryJump();
            }
        }

        private void HandleDashInputs()
        {
            if (IsTakingInputs())
            {
                if (Input.IsKeyDown(Keys.KEY_V)) TryDash();
            }
        }

        private void TryJump()
        {
            if (jumpCounter < 2 && jumpCooldownTimer <= 0.0f)
            {
                jumpCounter++;
                jumpCooldownTimer = jumpCooldown;
                jumpDurationTimer = jumpDuration;

                groundCheckLocked = true;
                groundContactCount = 0;
                grounded = false;

                if (jumpCounter == 1)
                    playerMovementState = MovementState.Jumping;
                else
                    playerMovementState = MovementState.DoubleJumping;

                Jump();
            }
        }

        void Jump()
        {
            Vector3 camForward = new Vector3();
            if (camera != null)
            {
                camForward = camera.transform.RotationQuat * Vector3.Forward; // Get camera forward direction
                camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
                camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera
            }
            Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
            Vector3 moveDir = (camForward * input.z + camRight * input.x).Normalize();
            Vector3 jumpDir = moveDir * jumpHorizontalForce;
            rigidBody.Velocity = new Vector3(jumpDir.x, jumpVerticalForce, jumpDir.z);            
        }

        void TryDash()
        {
            if (dashCooldownTimer <= 0.0f)
            {
                dashCooldownTimer = dashCooldown;
                dashDurationTimer = dashDuration;

                if (grounded)
                    playerMovementState = MovementState.GroundDash;
                else
                    playerMovementState = MovementState.AirDash;

                BeginGroundDash();
            }
        }

        Vector3 ComputeFlatDashDir()
        {
            Vector3 forward = transform.Forward;
            forward.y = 0f; // flatten to horizontal plane
            return forward.Normalize();
            //bool hasInput = input.SquareMagnitude() > 0.0001f;
            //if (hasInput)
            //    input = input.Normalize();

            //// --- NO INPUT OR FORCED FORWARD ---
            //if (!useMoveDir || !hasInput)
            //{
            //    Vector3 forward = transform.Forward;
            //    forward.y = 0f;
            //    return (dashDefaultBackwards ? -forward : forward).Normalize();
            //}

            //// --- CAMERA-RELATIVE DASH ---
            //if (camera != null)
            //{
            //    Vector3 camForward = camera.transform.RotationQuat * Vector3.Forward;
            //    camForward.y = 0f;
            //    camForward = camForward.Normalize();

            //    Vector3 camRight = Vector3.Cross(Vector3.Up, camForward);

            //    Vector3 dashDir =
            //        camForward * input.z +
            //        camRight * input.x;

            //    dashDir.y = 0f;
            //    return dashDir.Normalize();
            //}

            //// --- TRANSFORM-RELATIVE FALLBACK ---
            //Vector3 moveDir =
            //    transform.Forward * input.z +
            //    transform.Right * input.x;

            //moveDir.y = 0f;
            //return moveDir.Normalize();
        }

        void BeginGroundDash()
        {
            playerMovementState = MovementState.GroundDash;

            // Use player’s facing direction
            dashDir = ComputeFlatDashDir();

            // Rotate the player to face the dash direction instantly
            Vector3 faceDir = dashDir;
            faceDir.y = 0f; // flatten to horizontal
            if (faceDir.SquareMagnitude() > 0.0001f)
            {
                transform.RotationQuat = Quaternion.LookRotation(faceDir.Normalize(), Vector3.Up);
            }

            allowedDashDistance = dashDistance;


            // Optional: prevent dashing into walls
            if (Physics.Raycast(transform.Position + new Vector3(0f, 2f, 0f), dashDir * 1000f, out RayCastHit dashHitInfo, LayerMask.GetMask("Environment"), QueryTriggerInteraction.UseGlobal))
            {
                if (allowedDashDistance >= dashHitInfo.distance)
                {
                    allowedDashDistance = dashHitInfo.distance * 0.98f;
                }
            }

            dashDurationTimer = Math.Max(0.0001f, dashStartDuration);
        }

        void Dash(Vector3 moveDir, float dt)
        {
            float dashSpeed = dashDistance / Math.Max(0.0001f, dashStartDuration);
            float distanceThisFrame = dashSpeed * dt;
            float moveAmount = Math.Min(allowedDashDistance, distanceThisFrame);

            if (allowedDashDistance >= 0f)
                allowedDashDistance -= moveAmount;
            else
                dashCooldownTimer = dashCooldown;

            Vector3 dashVel = moveDir * moveAmount;
            finalMove = playerMovementState == MovementState.GroundDash
                ? new Vector3(dashVel.x, dashVel.y + velocity.y, dashVel.z)
                : dashVel;

            transform.Position += finalMove;

            // Smoothly rotate towards dash direction
            Vector3 face = dashDir;
            face.y = 0f;
            if (face.SquareMagnitude() > 0.0001f)
            {
                transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, Quaternion.LookRotation(face.Normalize(), Vector3.Up), 20f * dt);
            }
        }

        private void TryAttack()
        {            
            attackQueued = true;
        }

        private void InitializeAttacks()
        {
            attackHitboxes.Clear();
            attackHitboxes.Add(FindGameObjectsWithTag(attackHitboxNames[0])[0]?.As<GeneralHitbox>());
            attackHitboxes[0].HitBoxListeners += Attack1;

            attackHitboxes.Add(FindGameObjectsWithTag(attackHitboxNames[1])[0]?.As<GeneralHitbox>());
            attackHitboxes[1].HitBoxListeners += Attack1;

            attackHitboxes.Add(FindGameObjectsWithTag(attackHitboxNames[2])[0]?.As<GeneralHitbox>());
            attackHitboxes[2].HitBoxListeners += Attack1;

            //TurnOffHitboxes();
        }
        private void InitializeInternalReferences()
        {
            playerModel = gameObject.FindGameObjectWithName("RootNode"); if (playerModel == null) SliceLog.Warn("PlayerController cannot find RootNode");
            animator = playerModel?.GetComponent<Animator>(); if (playerModel == null) SliceLog.Warn("PlayerController cannot find Animator");
            audio = gameObject.GetComponent<AudioSource>(); if (playerModel == null) SliceLog.Warn("PlayerController cannot find AudioSource");
            rigidBody = GetComponent<RigidBody>(); if (playerModel == null) SliceLog.Warn("PlayerController cannot find RigidBody");            
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
                grounded = (groundContactCount > 0); ;
                if (grounded)
                {
                    jumpCounter = 0;
                }
            }
        }

        public override void OnCollideEnter(uint other)
        {
            if (IsGround(other) && !groundCheckLocked)
            {
                groundContactCount++;
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (IsGround(other) && !groundCheckLocked)
            {
                groundContactCount--;
                if (groundContactCount < 0)
                    groundContactCount = 0;
            }
        }
        private bool IsGround(uint id)
        {
            if (gameObject.FindGameObjectWithID(id).tag == "Ground")
            {
                return true;
            }
            return false;
        }

        bool IsTakingInputs()
        {
            if (playerControlState != ControlState.Gameplay)
            {
                return false;
            }

            if (playerCombatState != CombatState.None)
            {
                return false;
            }

            bool inputtable;
            if (playerMovementState == MovementState.Idle || playerMovementState == MovementState.Walking || playerMovementState == MovementState.Falling || playerMovementState == MovementState.Jumping)
            {
                inputtable = true;
            }
            else
            {
                inputtable = false;
            }
            return inputtable;
        }
        #endregion
    }
}
using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using static SliceEngine.GeneralHitbox;


namespace SliceEngine
{
    public class PlayerController : Entity, IInitializable
    {
        // Debug Mode
        public bool debugMode = false;
        bool movementStateChanged = false;

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
            GroundAttack,
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

        //public float attackResetTime = 1f;
        private float attackResetTimer = 0f;
        public float attackRecoveryDuration = 0.5f;
        public float attack1Delay, attack2Delay, attack3Delay;
        private int attackCounter = 0;

        // Lunging (moving when attacking)
        float lungeTimer = 0f;
        public float lungeDuration = 0.5f;
        public float lungeSpeed = 5.0f;

        // Plunge (ground slam when attacking while midair)
        public float plungeVerticalForce = 20.0f;
        public float plungeTerminalVelocity = 40.0f;
        public float plungeAttackDuration = 0.25f;
        public float plungeRecoveryDuration = 0.25f;
        public float plungeMinDistance = 2.0f;

        // Ground Check
        public float groundCheckDelay = 0.1f;
        public bool grounded;
        public int groundContactCount;

        // Jumps
        int jumpCounter = 0;
        public int jumpMax = 2;
        public float jumpVerticalForce = 15.0f;
        public float jumpHorizontalForce = 7.5f;
        public float jumpDuration = 0.25f;
        public float jumpCooldown = 0.25f;
        public float jumpLandDuration = 0.25f;
        float jumpDurationTimer = 0.0f;
        float jumpCooldownTimer = 0.0f;
        float jumpLandTimer = 0.0f;

        public float fallTimeThreshold = 0.25f;
        float fallTimeTimer = 0.0f;
        public float fallSpeed = 20.0f;

        // Movement
        Vector3 input;
        //Vector3 finalMove;
        public float moveAcceleration = 100.0f;
        public float moveDeceleration = 100.0f;
        public float rotationSpeed = 45.0f;
        public float strafeMultiplier = 0.75f;

        // Dash
        public float dashDuration = 0.25f;
        public float dashCooldown = 0.75f;
        float dashDurationTimer = 0.0f;
        float dashCooldownTimer = 0.0f;
        public int dashArrayIndex = 3;

        public float dashSpeed = 20.0f;
        Vector3 dashDir = Vector3.Zero;
        public float iFrameDuration = 0.2f;
        public float flickerDuration = 0.05f;
        public bool iFrames = false;

        public void Initialize()
        {
            camera = Bootstrap.CameraController; if (camera == null) SliceLog.Warn("PlayerController cannot find camera");

            Bootstrap.HUDManager.SetHealth(currentHealth / maxHealth);
        }

        public override void OnCreate()
        {
            InitializeInternalReferences();
            InitializeAttacks();
        }

        public override void OnUpdate(float dt)
        {
            MovementState prevMoveState = playerMovementState;
            CombatState prevCombatState = playerCombatState;
            CurrentAttack prevAttack = playerCurrentAttack;

            HandleInputs();
            GroundCheck();

            UpdateTimers(dt);
            UpdateAttacks(dt);
            UpdateStates();
            UpdateAnimator();


            if (prevMoveState != playerMovementState)
            {
                SliceLog.Log(playerMovementState.ToString());
                OnMovementStateChange();
            }

            if (prevCombatState != playerCombatState)
            {
                SliceLog.Log(playerCombatState.ToString());
                OnCombatStateChange();
            }

            if (prevAttack != playerCurrentAttack)
            {
                SliceLog.Log(playerCurrentAttack.ToString());
                OnCombatStateChange();
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            UpdateMovements(dt);

        }

        void OnMovementStateChange()
        {
            movementStateChanged = true;
        }

        void OnCombatStateChange()
        {

        }

        void OnCurrentAttackChange()
        {

        }

        private void HandleInputs()
        {
            input = Vector3.Zero;

            if (!IsTakingInputs())
            {
                return;
            }

            HandleMovementInputs();

            HandleAttackInputs();

            HandleDashInputs();

            HandleJumpInputs();

            if (input.SquareMagnitude() > 1f) input = input.Normalize();
        }

        #region Attacking
        private void ExecuteAttack()
        {
            if (playerCombatState != CombatState.Attacking && playerMovementState != MovementState.Plunging && grounded)
            {
                playerCombatState = CombatState.Attacking;

                // Ground attacking
                playerCurrentAttack = CurrentAttack.GroundAttack;
                attackCounter++;
                if (attackCounter > 3) attackCounter = 1;
                attackTimer = attackDuration[(int)playerCurrentAttack];

                switch (attackCounter)
                {
                    case 1:
                        StartCoroutine(AttackDelay(attack1Delay, () => attackHitboxes[0].TurnOn()));

                        AudioSettings.PlaySFX("A1");

                        playerMovementState = MovementState.Lunging;
                        lungeTimer = lungeDuration;
                        break;
                    case 2:
                        StartCoroutine(AttackDelay(attack1Delay, () => attackHitboxes[1].TurnOn()));

                        if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                        {
                            AudioSettings.PlaySFX("A2");
                        }

                        playerMovementState = MovementState.Lunging;
                        lungeTimer = lungeDuration;
                        break;
                    case 3:
                        StartCoroutine(AttackDelay(attack1Delay, () => attackHitboxes[2].TurnOn()));

                        if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0)
                        {
                            AudioSettings.PlaySFX("A3");
                        }
                        break;
                    default:
                        break;
                }
            }
            else if (playerCombatState != CombatState.Attacking && playerMovementState != MovementState.Plunging)
            {
                Console.WriteLine("Trying to execute attack");
                RayCastHit hitInfo;
                // Check if can plunge by raycasting down to see distance to ground
                bool hit = Physics.Raycast(transform.Position + new Vector3(0, 1, 0), new Vector3(0, -1, 0) * 1000f, out hitInfo, LayerMask.GetMask("Environment"), QueryTriggerInteraction.UseGlobal);
                if (hit)
                    Physics.DebugDrawRay(transform.Position + new Vector3(0, 1, 0), new Vector3(0, -1, 0), 5.0f);
                if (hit)
                {
                    //Console.WriteLine("It hit something");
                    GameObject objHit = FindGameObjectWithID(hitInfo.transform.gameObject.mID);
                    if (objHit == null)
                    {
                        Console.WriteLine("Obj hit is null");
                    }
                    // Only check distance if its a ground obj
                    else if (objHit.tag == "Ground")
                    {
                        // if its too close to the ground then dont let it plunge
                        if (hitInfo.distance <= plungeMinDistance)
                        {
                            //Console.WriteLine("Not high enough");
                            return;
                        }
                        else
                        {
                            Console.WriteLine($"Distance : {hitInfo.distance}");
                        }
                    }
                    else
                    {
                        Console.WriteLine($"It hit smth that isnt ground: {objHit.mID}");
                    }
                }
                else
                {
                    Console.WriteLine("Not hitting");
                }



                playerCombatState = CombatState.Attacking;
                playerMovementState = MovementState.Plunging;
                playerCurrentAttack = CurrentAttack.None;

                if (String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
                {
                    AudioSettings.PlaySFX("Plunge");
                }               
            }
        }
        private void TurnOffHitboxes()
        {
            foreach (var hb in attackHitboxes)
            {
                hb.TurnOff();
            }
        }

        public void StartAttackRecovery()
        {
            playerCombatState = CombatState.Recovery;
            attackResetTimer = 0f;

            TurnOffHitboxes();
        }
        private void AttackReset()
        {
            if (attackResetTimer >= attackRecoveryDuration)
            {
                if (playerCurrentAttack == CurrentAttack.GroundAttack)
                {
                    if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0 && (String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") != 0))
                    {
                        if (animator.SafeToChange("AttackToIdle1"))
                            animator.SetBool("AttackToIdle1", true);
                    }
                    if (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0 && (String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") != 0))
                    {
                        if (animator.SafeToChange("AttackToIdle2"))
                            animator.SetBool("AttackToIdle2", true);
                    }
                    if (String.Compare(animator.GetCurrAnimName(), "Attack3") == 0 && (String.Compare(animator.GetCurrAnimName(), "AttackToIdle3") != 0))
                    {
                        if (animator.SafeToChange("AttackToIdle3"))
                            animator.SetBool("AttackToIdle3", true);
                    }
                }
                else if (playerCurrentAttack == CurrentAttack.PlungeLand)
                {                    
                    if (input != Vector3.Zero)
                    {
                        if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") == 0 && (String.Compare(animator.GetCurrAnimName(), "PlungeToWalk") != 0))
                        {
                            if (animator.SafeToChange("PlungeToWalk"))
                                animator.SetBool("PlungeToWalk", true);                            
                        }
                        playerMovementState = MovementState.Walking;
                    }
                    else
                    {
                        if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") == 0 && (String.Compare(animator.GetCurrAnimName(), "PlungeToIdle") != 0))
                        {
                            if (animator.SafeToChange("PlungeToIdle"))
                                animator.SetBool("PlungeToIdle", true);
                        }
                        playerMovementState = MovementState.Idle;
                    }                    
                }

                playerCurrentAttack = CurrentAttack.None;
                attackCounter = 0;
            }
        }

        private void Attack1(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                Console.WriteLine($"Attacking enemy in attack 1");
                enemy.TakeDamage(attackDamageValues[attackCounter]);
            }
        }
        private void Attack2(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                Console.WriteLine($"Attacking enemy in attack 2");
                enemy.TakeDamage(attackDamageValues[attackCounter]);
            }
        }
        private void Attack3(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attackDamageValues[attackCounter]);
            }
        }
        private IEnumerator AttackDelay(float delay, Action action)
        {
            yield return new WaitForSeconds(delay);
            action.Invoke();
        }
        void EndAttackState()
        {
            attackTimer = 0f;
            playerCurrentAttack = CurrentAttack.None;
            attackQueued = false;
            playerCombatState = CombatState.None;
            attackResetTimer = 0.0f;
            TurnOffHitboxes();

            // Clear movement bursts
            playerMovementState = grounded ? MovementState.Idle : MovementState.Falling;

        }

        private void TryAttack()
        {
            attackQueued = true;
        }

        private void InitializeAttacks()
        {
            attackHitboxes.Clear();
            //attackHitboxes.Add(FindGameObjectWithName(attackHitboxNames[0])?.As<GeneralHitbox>());
            //attackHitboxes[0].HitBoxListeners += Attack1;

            //attackHitboxes.Add(FindGameObjectWithName(attackHitboxNames[1])?.As<GeneralHitbox>());
            //attackHitboxes[1].HitBoxListeners += Attack2;

            //attackHitboxes.Add(FindGameObjectWithName(attackHitboxNames[2])?.As<GeneralHitbox>());
            //attackHitboxes[2].HitBoxListeners += Attack3;

            for (int i = 0; i < attackHitboxNames.Count; i++)
            {
                attackHitboxes.Add(FindGameObjectWithName(attackHitboxNames[i])?.As<GeneralHitbox>());

                HitBoxTriggerEvent attackAction = null;

                switch (i)
                {
                    case 0:
                        attackAction = Attack1;
                        break;
                    case 1:
                        attackAction = Attack2;
                        break;
                    case 2:
                        attackAction = Attack3;
                        break;
                    case 3:
                        attackAction = DashAttack;
                        break;
                }
                attackHitboxes[i].HitBoxListeners += attackAction;
            }
            Console.WriteLine($"Found {attackHitboxes.Count} hitboxes");
            TurnOffHitboxes();
        }
        private void InitializeInternalReferences()
        {
            playerModel = gameObject.FindGameObjectWithName("RootNode"); if (playerModel == null) SliceLog.Warn("PlayerController cannot find RootNode");
            animator = playerModel?.GetComponent<Animator>(); if (playerModel == null) SliceLog.Warn("PlayerController cannot find Animator");
            audio = gameObject.GetComponent<AudioSource>(); if (playerModel == null) SliceLog.Warn("PlayerController cannot find AudioSource");
            rigidBody = GetComponent<RigidBody>(); if (playerModel == null) SliceLog.Warn("PlayerController cannot find RigidBody");
        }

        #endregion

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

            if (playerCombatState != CombatState.Attacking)
                attackResetTimer = (attackResetTimer <= attackRecoveryDuration) ? attackResetTimer + dt : 0.0f;

            if (!grounded) fallTimeTimer += dt;
            else fallTimeTimer = 0.0f;
        }
        private void UpdateMovements(float dt)
        {
            Vector3 camForward = new Vector3();
             Vector3 velTemp = rigidBody.Velocity;
            //finalMove = Vector3.Zero;
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
                Dash();

                Vector3 dashVel = dashDir * dashSpeed;
                float yVel = 0;//playerMovementState == MovementState.GroundDash ? 0 : rigidBody.Velocity.y;
                rigidBody.Velocity = new Vector3(dashVel.x, yVel, dashVel.z);

            }
            else if (playerMovementState == MovementState.Jumping || playerMovementState == MovementState.Falling)
            {
                moveDirInput *= strafeMultiplier;
            }
            else if (playerMovementState == MovementState.Lunging)
            {
                Vector3 lungeDir = transform.Forward;
                lungeDir.y = 0.0f;
                lungeDir = lungeDir.Normalize();
                Vector3 lungeVel = lungeDir * lungeSpeed;
                rigidBody.Velocity = new Vector3(lungeVel.x, rigidBody.Velocity.y, lungeVel.z);
            }
            else if (playerMovementState == MovementState.Plunging && !grounded)
            {
                velTemp.x = 0.0f;
                if (rigidBody.Velocity.y < -plungeTerminalVelocity)
                {
                    velTemp.y -= plungeVerticalForce * dt;
                }
                else
                {
                    velTemp.y = -plungeTerminalVelocity;
                }
                
                rigidBody.Velocity = velTemp;
            }

            if (playerMovementState == MovementState.Idle || playerMovementState == MovementState.Walking || playerMovementState == MovementState.Falling || playerMovementState == MovementState.Jumping)
            {
                if (playerCombatState == CombatState.Attacking) return;
                // Normal locomotion
                if (moveDirInput.SquareMagnitude() > 0.0001f)
                {
                    Quaternion targetRot = Quaternion.LookRotation(moveDirInput, Vector3.Up);
                    float scaledRotSpeed = rotationSpeed;
                    transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRot, scaledRotSpeed * Time.deltaTime);
                    
                }
                Vector3 horizontal = moveDirInput * movementSpeed;
                rigidBody.Velocity = new Vector3(horizontal.x, rigidBody.Velocity.y, horizontal.z);
            }
        }

        void UpdateAttacks(float dt)
        {
            if (attackQueued)
            {
                ExecuteAttack();
                attackQueued = false;
            }
            AttackReset();
        }

        void UpdateStates()
        {
            //Console.WriteLine($"Current movement state: {playerMovementState.ToString()}");
            switch (playerMovementState)
            {
                case MovementState.Idle:
                    if (!grounded && jumpDurationTimer <= 0.0f && fallTimeTimer > fallTimeThreshold)
                    {
                        playerMovementState = MovementState.Falling;
                    }
                    else if (input != Vector3.Zero)
                    {
                        playerMovementState = MovementState.Walking;
                    }

                    break;
                case MovementState.Walking:
                    if (!grounded && jumpDurationTimer <= 0.0f && fallTimeTimer > fallTimeThreshold)
                    {
                        playerMovementState = MovementState.Falling;
                    }
                    else if (input == Vector3.Zero)
                    {
                        playerMovementState = MovementState.Idle;
                    }
                    break;
                case MovementState.Jumping:
                    if (!grounded && jumpDurationTimer <= 0.0f && fallTimeTimer > fallTimeThreshold)
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
                        fallTimeTimer = 0.0f;
                    }
                    break;
                case MovementState.Landing:
                    if (jumpLandTimer <= 0.0f && (playerCurrentAttack != CurrentAttack.PlungeLand))
                    {
                        playerMovementState = input == Vector3.Zero ? MovementState.Idle : MovementState.Walking;
                    }                    
                    break;
                case MovementState.GroundDash:
                    if (dashDurationTimer <= 0.0f)
                    {
                        Vector3 vel = rigidBody.Velocity;
                        vel.x *= 0.1f;
                        vel.z *= 0.1f;
                        rigidBody.Velocity = vel;
                        attackHitboxes[dashArrayIndex].TurnOff();
                        playerMovementState = MovementState.Idle;
                    }
                    break;
                case MovementState.AirDash:
                    if (dashDurationTimer <= 0.0f)
                    {
                        Vector3 vel = rigidBody.Velocity;
                        vel.x *= 0.1f;
                        vel.z *= 0.1f;
                        rigidBody.Velocity = vel;

                        Console.WriteLine($"Transitioning to falling from {playerMovementState.ToString()}");

                        attackHitboxes[dashArrayIndex].TurnOff();
                        playerMovementState = MovementState.Falling;
                    }
                    break;
                case MovementState.Lunging:
                    if (lungeTimer <= 0.0f)
                    {
                        Console.WriteLine("Changing movement state from lunging");
                        playerMovementState = grounded ? MovementState.Idle : MovementState.Falling;
                    }
                    break;
                case MovementState.Plunging:
                    if (grounded)
                    {
                        playerMovementState = MovementState.Landing;
                        playerCurrentAttack = CurrentAttack.PlungeLand;
                        attackTimer = plungeAttackDuration;
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
                    if (playerMovementState == MovementState.Lunging)
                    {
                        playerMovementState = grounded ? MovementState.Idle : MovementState.Falling;
                    }
                    break;
                case CombatState.Attacking:
                    if (attackTimer <= 0.0f)
                    {
                        playerCombatState = CombatState.Recovery;
                        attackResetTimer = 0.0f;
                    }
                    break;
                case CombatState.Recovery:
                    if (attackResetTimer >= attackRecoveryDuration)
                    {
                        playerCombatState = CombatState.None;
                    }
                    break;
                case CombatState.Hitstun:
                    break;
                default:
                    break;
            }

            switch (playerCurrentAttack)
            {
                case CurrentAttack.PlungeLand:
                    if (attackTimer > 0.0f)
                    {
                        playerCombatState = CombatState.Attacking;                        
                    }
                    break;

                default:
                    break;
            }
        }

        void ResetMovementAnimator()
        {
            animator.SetBool("Idle", false);
            animator.SetBool("Walk", false);
            animator.SetBool("JumpLoop", false);
            animator.SetBool("Fall", false);
            animator.SetBool("Land", false);
        }

        private IEnumerator iFrameAnimation(float duration)
        {
            float timer = 0.0f;
            float flickerTimer = 0.0f;
            bool flicker = false;
            while (timer < duration)
            {
                if (flickerTimer >= flickerDuration)
                {
                    playerModel.As<PlayerAnimatorEvents>().SetModelVisible(flicker);
                    flicker = !flicker;
                    Console.WriteLine($"Flicker timer {flickerTimer}");
                    flickerTimer = 0.0f;
                }

                Console.WriteLine($"total timer {timer}");

                timer += Time.deltaTime;
                flickerTimer += Time.deltaTime;
                yield return null;
            }

            //Set visible at the end
            playerModel.As<PlayerAnimatorEvents>().SetModelVisible(true);
            iFrames = false;
        }

        void UpdateAnimator()
        {

            if (animator == null)
                return;
           // Console.WriteLine($"Current anim name {animator.GetCurrAnimName()}");

            if (movementStateChanged)
            {
                ResetMovementAnimator();
                movementStateChanged = false;
            }

            switch (playerMovementState)
            {
                case MovementState.Idle:
                    if (animator.SafeToChange("Idle") && (String.Compare(animator.GetCurrAnimName(), "Idle") != 0))
                        animator.SetBool("Idle", true);
                    break;
                case MovementState.Walking:
                    if (animator.SafeToChange("Walk") && (String.Compare(animator.GetCurrAnimName(), "Walk") != 0))
                        animator.SetBool("Walk", true);
                    break;
                case MovementState.Jumping:
                    if (animator.SafeToChange("JumpLoop") && (String.Compare(animator.GetCurrAnimName(), "JumpLoop") != 0))
                        animator.SetBool("JumpLoop", true);
                    break;
                case MovementState.DoubleJumping:
                    if (animator.SafeToChange("DoubleJump") && (String.Compare(animator.GetCurrAnimName(), "DoubleJump") != 0))
                        animator.SetBool("DoubleJump", true);
                    //Console.WriteLine("AirDashing now");
                    break;
                case MovementState.Falling:
                    if (animator.SafeToChange("Fall") && (String.Compare(animator.GetCurrAnimName(), "Fall") != 0))
                        animator.SetBool("Fall", true);
                    break;
                case MovementState.Landing:
                    if (animator.SafeToChange("Land") && (String.Compare(animator.GetCurrAnimName(), "Land") != 0))
                        animator.SetBool("Land", true);
                    break;
                case MovementState.GroundDash:
                    {
                        bool hasInput = input.SquareMagnitude() > 0.0001f;
                         if (hasInput)
                        {
                            // transition to forward dash instead of back dash
                            if (animator.SafeToChange("Dash") && (String.Compare(animator.GetCurrAnimName(), "Land") != 0))
                            {
                                //Console.WriteLine("Setting it again");
                                animator.SetBool("Dash", true);
                            }
                        }
                        else
                        {
                            if (animator.SafeToChange("BackDash") && (String.Compare(animator.GetCurrAnimName(), "Land") != 0))
                                animator.SetBool("BackDash", true);
                        }
                    }
                    break;

                case MovementState.AirDash:
                    {
                        bool hasInput = input.SquareMagnitude() > 0.0001f;
                        if (hasInput)
                        {
                            // transition to forward dash instead of back dash
                            if (animator.SafeToChange("Dash") && (String.Compare(animator.GetCurrAnimName(), "Land") != 0))
                            { 
                                Console.WriteLine("Setting it again");
                                animator.SetBool("Dash", true);
                            }
                        }
                        else
                        {
                            if (animator.SafeToChange("BackDash") && (String.Compare(animator.GetCurrAnimName(), "Land") != 0))
                                animator.SetBool("BackDash", true);
                        }
                    }
                    break;

                case MovementState.Lunging:
                    break;

                case MovementState.Plunging:
                    if (animator.SafeToChange("Plunge") && (String.Compare(animator.GetCurrAnimName(), "Plunge") != 0))
                        animator.SetBool("Plunge", true);
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

            switch (playerCurrentAttack)
            {
                case CurrentAttack.None:
                    break;
                case CurrentAttack.GroundAttack:
                    switch (attackCounter)
                    {
                        case 1:
                            {
                                if (String.Compare(animator.GetCurrAnimName(), "Attack1") != 0)
                                    animator.SetBool("Attack1", true);
                                break;
                            }
                        case 2:
                            {
                                if (String.Compare(animator.GetCurrAnimName(), "Attack2") != 0 && (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0))
                                {
                                    animator.SetBool("Attack2", true);
                                }
                                break;
                            }
                        case 3:
                            {
                                if (String.Compare(animator.GetCurrAnimName(), "Attack3") != 0 && (String.Compare(animator.GetCurrAnimName(), "Attack2") == 0))
                                {
                                    animator.SetBool("Attack3", true);
                                }
                                break;
                            }
                    }
                    break;
                case CurrentAttack.PlungeLand:
                    if (String.Compare(animator.GetCurrAnimName(), "PlungeLand") != 0)
                        animator.SetBool("PlungeLand", true);
                    break;
                default:
                    break;
            }

        }

        private void HandleMovementInputs()
        {
            if (Input.IsKeyDown(Keys.KEY_W)) input += new Vector3(0f, 0f, 1f);
            else if (Input.IsKeyDown(Keys.KEY_S)) input += new Vector3(0f, 0f, -1f);

            // Sideways movement 
            if (Input.IsKeyDown(Keys.KEY_A)) input += new Vector3(1f, 0f, 0f);
            else if (Input.IsKeyDown(Keys.KEY_D)) input += new Vector3(-1f, 0f, 0f);
        }

        private void HandleAttackInputs()
        {
            if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
        }

        private void HandleJumpInputs()
        {
            if (Input.IsKeyPressed(Keys.KEY_SPACEBAR)) TryJump();
        }

        private void HandleDashInputs()
        {
            if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_RIGHT)) TryDash();
        }

        private void TryJump()
        {
            // no jumping when attacking
            if (playerCombatState != CombatState.None)
                return;

            if (jumpCounter < 2 && jumpCooldownTimer <= 0.0f)
            {
                jumpCounter++;
                jumpCooldownTimer = jumpCooldown;
                jumpDurationTimer = jumpDuration;
                Console.WriteLine($"jump Counter {jumpCounter} and movementState: {playerMovementState.ToString()}");
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

                Console.WriteLine($"Dashing now, prev state is : {playerMovementState.ToString()}");

                if (grounded)
                    playerMovementState = MovementState.GroundDash;
                else
                    playerMovementState = MovementState.AirDash;

                dashDir = ComputeFlatDashDir(true);

                Console.WriteLine($"Dashing now, after state is : {playerMovementState.ToString()}");
            }
        }

        void Dash()
        {
            bool hasInput = input.SquareMagnitude() > 0.0001f;

            if (hasInput)
            {
                dashDir = ComputeFlatDashDir(true);

                if (dashDir.SquareMagnitude() > 0.0001f)
                {
                    transform.RotationQuat = Quaternion.LookRotation(dashDir, Vector3.Up);
                }
            }
            attackHitboxes[dashArrayIndex].TurnOn();
        }
        void DashAttack(GameObject target = null)
        {
            Console.WriteLine((target == null).ToString());
            EnemyBase enemy = target?.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attackDamageValues[dashArrayIndex]);
                dashDurationTimer = 0f;
                Console.WriteLine("Dealing damage using dash");
            }
        }

        private void GroundCheck()
        {
            grounded = (groundContactCount > 0); ;
            if (grounded)
            {
                if (playerMovementState != MovementState.Jumping && playerMovementState != MovementState.Falling)
                    jumpCounter = 0;
                //Console.WriteLine("Resetting jump counter");
            }
        }

        public override void OnCollideEnter(uint other)
        {
            if (IsGround(other))
            {
                //Console.WriteLine($"Colliding with {other}");
                groundContactCount++;
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (IsGround(other))
            {
              //  Console.WriteLine($"Exit Colliding with {other}");

                groundContactCount--;
                if (groundContactCount < 0)
                {
                    groundContactCount = 0;
                    Console.WriteLine("Gonna reset jump counter");
                }
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

            if (playerCombatState == CombatState.Attacking)
            {
                return false;
            }

            bool inputtable;
            if (playerMovementState == MovementState.Idle || playerMovementState == MovementState.Walking || playerMovementState == MovementState.Falling || playerMovementState == MovementState.Jumping || playerMovementState == MovementState.GroundDash || playerMovementState == MovementState.AirDash)
            {
                inputtable = true;
            }
            else
            {
                inputtable = false;
            }
            return inputtable;
        }

        Vector3 ComputeFlatDashDir(bool useMoveDir)
        {
            bool hasInput = input.SquareMagnitude() > 0.0001f;
            Vector3 normInput = hasInput ? input.Normalize() : Vector3.Zero;

            if (!useMoveDir || !hasInput)
            {
                Vector3 forward = transform.Forward;
                forward.y = 0f;
                return -forward.Normalize();
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

        public void SetPlayerLock(bool lockPlayer)
        {
            if (lockPlayer)
            {
                
                playerControlState = ControlState.Cutscene;

                
                input = Vector3.Zero;

                
                if (grounded)
                {
                    playerMovementState = MovementState.Idle;
                }
            }
            else
            {
                // Return control to the player
                playerControlState = ControlState.Gameplay;
            }
        }
        
        public void TeleportPlayer(Vector3 pos)
        {
            EndAttackState();
            transform.Position = pos;
        }

        #endregion
    }
}
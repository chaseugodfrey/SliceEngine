using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using SliceEngine;
using static System.Runtime.CompilerServices.RuntimeHelpers;


namespace SliceEngine
{

    public class PlayerController : Entity, IInitializable
    {
        public CameraController camRig;
        public Animator animator;
        private bool dead = false;

        public int attack1_Damage = 20;
        public int attack2_Damage = 30;
        public int attack3_Damage = 50;
        public int plunge_Damage = 60;

        public float healthRegenRate = 0.5f;
        public float healthRegenCooldown = 5f;

        public float walkSpeed = 3.5f;
        public float rotationSpeed = 12f;

        public float jumpHeight = 1.4f;
        public float gravity = -9.81f;
        public float groundedGravity = -2f;
        public float coyoteTime = 0.08f;
        public float jumpBuffer = 0.10f;
        private float jumpDelay = 0.00f;
        private int jumpCounter = 0;

        private bool enableDoubleJump = true;
        public float doubleJumpHeight = 1.2f;
        private bool resetYOnDoubleJump = true;

        // --- Ground / landing debounce ----
        private float minAirTimeForLanding = 0.06f;   // tune 0.04 - 0.12 to taste

        private float landCooldown = 0.08f;           // prevents immediate retriggering
        private bool grounded, groundCheckLocked;
        public float groundCheckDelay = 0.05f;
        private float groundCheckTimer = 0f;
        private float lastAirTime = float.NegativeInfinity;
        private float lastLandTime = float.NegativeInfinity;

        // ------------------ DASH ------------------------------------------------------------------------------------------
        private bool useBackdashAnimation = true;
        public float backDashDotThreshold = 0f;
        public string backDashTrigger = "BackDashStart";
        public string fwdDashTrigger = "DashStart";
        public string airDashTrigger = "AirDashStart";
        private bool dashDefaultBackwards = true;
        private bool rotateToDashDirection = false;

        public float dashDistance = 6f;
        public float dashStartDuration = 0.25f;
        public float dashCooldown = 0.6f;
        public bool dashUsesMoveDirection = true;
        public float groundDashUpAngleDeg = 0f;
        private GroundCheck groundCheck;

        public float airDashDistance = 5f;
        public float airDashDuration = 0.25f;
        public float airDashCooldown = 0.8f;
        public float airDashUpAngleDeg = 15f;

        // ------------------ ATTACK COMBO ------------------------------------------------------------------------------------------
        public string atk1Trigger = "Attack1";
        public string atk2Trigger = "Attack2";
        public string atk3Trigger = "Attack3";
        public string atkToIdle1Trigger = "AttackToIdle1";
        public string atkToIdle2Trigger = "AttackToIdle2";

        public float atk1Duration = 1.0f;
        public float atk1ChainLead = 0.30f;

        public float atk2Duration = 1.0f;
        public float atk2ChainLead = 0.30f;

        public float atk3Duration = 1.0f;
        public float atk3ChainLead = 0.30f;

        public float atk1LungeDistance = 2.5f;
        public float atk1LungeDuration = 0.15f;
        public float atk1LungeDelay = 0.05f;

        public float atk2LungeDistance = 3.0f;
        public float atk2LungeDuration = 0.18f;
        public float atk2LungeDelay = 0.05f;

        public float atk3ImpulseDelay = 0.08f;
        public float atk3ForwardSpeed = 7.0f;
        public float atk3UpwardSpeed = 8.0f;
        public float atk3ForwardDuration = 0.45f;
        public float atk3UpwardDuration = 0.30f;

        // ------------------ PLUNGE ATTACK ------------------------------------------------------------------------------------------
        public float plungeDelay = 0.15f;
        public float plungeForwardSpeed = 8f;
        public float plungeDownwardSpeed = 15f;
        public float plungeDuration = 0.5f;
        public string plungeTrigger = "Plunge";
        public float heightModifierPerOneExtra = .05f;
        public float heightThreshold = 3.5f;
        private float _plungemodifierCount = 0f;
        private Vector3 _plungeStartPoint = new Vector3(0, 0, 0);

        // ------------------ HIT BOXES ------------------------------------------------------------------------------------------
        public Hitbox basicHB_1;
        public Hitbox basicHB_2;
        public Hitbox basicHB_3;
        public Hitbox plungeHB;

    // ------------------ TAUNT DEVICE ------------------------------------------------------------------------------------------
        public float teleportCooldown = 10f;
        bool _teleportCoolingDown = false;

        // ----------------- AUDIO ------------------------------------------------------------------------------------------
        private AudioSource walkAudioSource;
        public AudioSource slimeHitAudioSource;
        public bool canPlaySlimeHitSFX = true;

        // ------------------ INTERNALS ------------------------------------------------------------------------------------------
        private Vector3 velocity;
        private bool wasGrounded;
        private float lastGroundedTime;
        private float lastJumpPressedTime;
        private bool jumpRequested;
        private bool jumpImpulseApplied;
        private float jumpApplyAtTime;
        private bool doubleJumpAvailable;
        public bool canInput = true;

        // Dash runtime
        private bool isGroundDashing = false;
        private bool isAirDashing = false;
        private float dashTimer = 0f;
        private float groundDashCooldownUntil = 0f;
        private float airDashCooldownUntil = 0f;
        private Vector3 dashDir = Vector3.Zero;

        // Attack runtime
        private bool isAttacking = false;
        private int attackIndex = 0;
        private float attackTimer = 0f;
        private bool queuedNext = false;

        // Chain-window facing override
        private bool queuedFacingOverride = false;
        private Vector3 queuedFacing = Vector3.Zero;

        // Lunge runtime (Atk1 / Atk2)
        private bool isLunging = false;
        private float lungeTimer = 0f;
        private float lungeDuration = 0f;
        private float lungeDelay = 0f;
        private bool lungeStarted = false;
        private Vector3 lungeDir = Vector3.Zero;
        private float lungeSpeed = 0f;

        // Attack 3 Arc runtime
        private bool atk3ArcActive = false;
        private float atk3ArcTimer = 0f;
        private bool atk3ImpulseFired = false;
        private Vector3 atk3ArcDir = Vector3.Zero;
        private Vector3 atk3HorizVel = Vector3.Zero;
        private float atk3UpwardEndTime = 0f;

        // Plunge runtime 
        private bool isPlunging = false;
        private float plungeTimer = 0f;
        private Vector3 plungeDir = Vector3.Zero;
        private bool plungeImpulseStarted = false;

        // Health runtime
        private bool regeneratingHealth = false;

        // instance
        public static PlayerController instance;

        // Skip Frame
        private bool ignoreMovementThisFrame = false;

        public void Initialize()
        {
            camRig = Bootstrap.CameraController;
            if (camRig == null)
            {
                Console.WriteLine("Camera Var in player is EMPTY");
            }
        }
        public override void OnCreate()
        {
            lastJumpPressedTime = float.NegativeInfinity;
            lastGroundedTime = float.NegativeInfinity;

            //mainAudioSource = this.GetComponent<AudioSource>();
            bool groundedNow = grounded;
            wasGrounded = groundedNow;
            if (groundedNow) lastGroundedTime = Time.time;
            groundCheck = gameObject.FindGameObjectWithName("Ground Check")?.As<GroundCheck>();
            if (groundCheck == null) Console.WriteLine("No ground check found");

            doubleJumpAvailable = enableDoubleJump && !groundedNow;

            InitializeHitboxes();

            //StartCoroutine(UltiCooldown());
        }

        void Update()
        {
            if (ignoreMovementThisFrame)
            {
                ignoreMovementThisFrame = false; // eat 1 frame
                return;
            }
            GroundCheck();
            if (canInput)
            {
                HandleDashInput();   // RMB cancels attacks
                HandleAttackInput(); // LMB starts/queues
                //HandleDeviceInput(); // Device inputs
            }
            UpdateDash();        // Timers
            UpdateAttack();      // Timers
            HandleMovement();    // Movement depends on dash/attack
            HandleJump();        // Blocked during attacks
            UpdatePlunge();      // Timers
            UpdateHealth();
        }

        // -------------------- Health NOT USED ------------------------------------


        private float healthRegenCounter = 0f;
        //[SerializeField] private float regenBuffer = 5f;

        public void UpdateHealth()
        {
            if (currentHealth <= maxHealth)
            {
                if (!regeneratingHealth)
                {
                    if (healthRegenCounter < healthRegenCooldown)
                    {
                        healthRegenCounter += Time.deltaTime;
                    }
                    else
                    {
                        healthRegenCounter = 0f;
                        StartCoroutine(RegenHealth());
                    }
                }

            }
            else
            {
                StopRegen();
            }
        }

        public void StopRegen()
        {
            regeneratingHealth = false;
            healthRegenCounter = 0f;
        }

        IEnumerator RegenHealth()
        {
            //Debug.Log("Regenerating Health");
            regeneratingHealth = true;
            float count = 0f;
            //float buffercount = 0f;

            while (regeneratingHealth)
            {
                count += Time.deltaTime;
                if (count > 1f / healthRegenRate)
                {
                    Heal(1);
                    count = 0f;
                }
                yield return new WaitForSeconds(Time.deltaTime);
            }

            regeneratingHealth = false;

            yield break;
        }
        //------------- Force Warp -------------
        #region Force Warp

        public void ForceWarp(Vector3 worldPosition)
        {
            // block movement logic this frame
            ignoreMovementThisFrame = true;
            velocity = Vector3.Zero;
            isGroundDashing = false;
            isAirDashing = false;
            isPlunging = false;
            isAttacking = false;
            jumpRequested = false;
        }

        #endregion
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
                grounded = groundCheck.Grounded; // Gets grounded status from GroundCheck component
                if (grounded)
                {
                    jumpCounter = 0;
                }
            }
        }
        // -------------------- Movement ------------------------------------------------------------------------------------------
        #region Movement
        private Vector2 GetAxis()
        {
            Vector2 result = new Vector2(0f ,0f);
            if (Input.IsKeyDown(Keys.KEY_A)) result.x = 1f;
            else if (Input.IsKeyDown(Keys.KEY_D)) result.x = -1f;

            if (Input.IsKeyDown(Keys.KEY_W)) result.y = 1f; 
            else if (Input.IsKeyDown(Keys.KEY_S)) result.y = -1f;

            return result;
        }
        void HandleMovement()
        {
            Vector2 input = GetAxis();
            if (input.SquareMagnitude() > 1f) input.Normalize();

            Vector3 camFwd = camRig != null ? camRig.GetFlatAimDirection(transform) : Vector3.Forward;
            Vector3 camRight = Vector3.Cross(Vector3.Up, camFwd).Normalize();
            Vector3 moveDirInput = camFwd * input.y + camRight * input.x;

            float rawPlanarSpeed = moveDirInput.Magnitude() * walkSpeed;

            if (isGroundDashing || isAirDashing)
            {
                float dashSpeed = isGroundDashing ? dashDistance / Math.Max(0.0001f, dashStartDuration)
                                                  : airDashDistance / Math.Max(0.0001f, airDashDuration);
                Vector3 dashVel = dashDir * dashSpeed;
                Vector3 finalMove = isGroundDashing
                    ? new Vector3(dashVel.x, dashVel.y + velocity.y, dashVel.z)
                    : dashVel;
                transform.Position += finalMove * Time.deltaTime;


                if (rotateToDashDirection)
                {
                    Vector3 face = dashDir; face.y = 0f;
                    if (face.SquareMagnitude() > 0.0001f)
                        transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, Quaternion.LookRotation(face.Normalize(), Vector3.Up), 20f * Time.deltaTime);
                }
            }
            else if (isAttacking || isPlunging)
            {
                if (isPlunging && !plungeImpulseStarted)
                {
                    // no movement at all
                    // (skip calling Move with any Y to avoid CC grounded quirks)
                }
                else
                {
                    // existing behavior for attacks / active plunge impulse
                    Vector3 finalMove = new Vector3(0f, velocity.y, 0f);
                    transform.Position += finalMove * Time.deltaTime;
                }
            }
            else
            {
                // Normal locomotion
                if (moveDirInput.SquareMagnitude() > 0.0001f)
                {
                    Quaternion targetRot = Quaternion.LookRotation(moveDirInput, Vector3.Up);
                    float scaledRotSpeed = rotationSpeed;
                    transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRot, scaledRotSpeed * Time.deltaTime);
                }

                Vector3 horizontal = moveDirInput * walkSpeed;
                Vector3 finalMove = new Vector3(horizontal.x, velocity.y, horizontal.z);
                transform.Position += finalMove * Time.deltaTime;
            }

            animator.SetFloat("Speed", (isAttacking ? 0f : rawPlanarSpeed));
            //animator.SetFloat("YVel", velocity.y);
            //animator.SetBool("IsDashing", isGroundDashing || isAirDashing);
            animator.SetBool("IsAttacking", isAttacking);

            bool walkingNow = !isAttacking && !isGroundDashing && !isAirDashing && grounded && rawPlanarSpeed > 0.1f;

            if (walkAudioSource != null)
            {
                if (walkingNow)
                {
                    if (!walkAudioSource.IsPlaying)
                        walkAudioSource.Play();
                }
                else
                {
                    if (walkAudioSource.IsPlaying)
                        walkAudioSource.Stop();
                }
            }
        }

        // -------------------- Jump ------------------------------------------------------------------------------------------
        void HandleJump()
        {
            if (grounded) lastGroundedTime = Time.time;

            if (wasGrounded && !grounded)
            {
                lastAirTime = Time.time; // mark when airborne
                animator.SetBool("Grounded", false);
            }

            if (!wasGrounded && grounded)
            {
                // Only trigger landing if we were in the air at least minAirTimeForLanding
                bool longEnoughAir = (Time.time - lastAirTime) >= minAirTimeForLanding;
                bool cooldownOK = (Time.time - lastLandTime) >= landCooldown;

                if (longEnoughAir && cooldownOK)
                {
                    //AudioManager.instance.PlaySFX("Land");
                    lastLandTime = Time.time;

                    //if (animator)
                    //{
                    //    animator.ResetTrigger("Land");
                    //    animator.SetTrigger("Land");
                    //    animator.SetBool("Grounded", true);
                    //}
                }
                else
                {
                    // still update animator grounded bool so we don't get stuck in 'air' state
                    //animator.SetBool("Grounded", true);
                }

                doubleJumpAvailable = enableDoubleJump;
                jumpRequested = false;
                jumpImpulseApplied = false;
            }

            // Jump input disabled during attacks
            if (!isAttacking && Input.IsKeyDown(Keys.KEY_SPACEBAR))
                lastJumpPressedTime = Time.time;

            bool canCoyote = (Time.time - lastGroundedTime) <= coyoteTime;
            bool bufferedJump = (Time.time - lastJumpPressedTime) <= jumpBuffer;

            if (!grounded && !canCoyote)
            {
                if (!isAttacking && enableDoubleJump && doubleJumpAvailable && Input.IsKeyDown(Keys.KEY_SPACEBAR))
                {
                    DoDoubleJump();
                }
            }
            else
            {
                if (!isAttacking && !jumpRequested && bufferedJump && canCoyote)
                {
                    jumpRequested = true;
                    jumpImpulseApplied = false;
                    jumpApplyAtTime = Time.time + Math.Max(0f, jumpDelay);

                    lastJumpPressedTime = -999f;
                    lastGroundedTime = -999f;

                    //if (animator)
                    //{
                    //    animator.ResetTrigger("Land");
                    //    animator.SetTrigger("Jump");
                    //    animator.SetBool("Grounded", false);
                    //}
                }
            }

            // Apply jump impulse (delayed if any)
            if (!isGroundDashing && !isAirDashing && !isAttacking && jumpRequested && !jumpImpulseApplied && Time.time >= jumpApplyAtTime)
            {
                float jumpSpeed = (float)Math.Sqrt(jumpHeight * -2f * gravity);
                velocity.y = jumpSpeed;
                jumpImpulseApplied = true;
                //AudioManager.instance.PlaySFX("Jump");
            }

            // Gravity
            if (isAirDashing)
            {
                velocity.y = 0f;
            }
            else
            {
                if (isPlunging && !plungeImpulseStarted)
                {
                    velocity.y = 0f;            // keep perfectly suspended
                                                // IMPORTANT: do NOT add gravity this frame
                }
                else
                {
                    if (grounded && velocity.y < 0f) velocity.y = groundedGravity;
                    velocity.y += gravity * Time.deltaTime;
                }
            }

            wasGrounded = grounded;
        }

        void DoDoubleJump()
        {
            doubleJumpAvailable = false;
            jumpRequested = false;
            jumpImpulseApplied = true;

            if (resetYOnDoubleJump && velocity.y < 0f) velocity.y = 0f;

            float jumpSpeed = (float)Math.Sqrt(doubleJumpHeight * -2f * gravity);
            velocity.y = jumpSpeed;

            //AudioManager.instance.PlaySFX("DoubleJump");

            //if (animator)
            //{
            //    animator.ResetTrigger("Jump");
            //    animator.SetTrigger("DoubleJump");
            //    animator.SetBool("Grounded", false);
            //}
        }

        #endregion

        // -------------------- Dash ------------------------------------------------------------------------------------------
        #region Dash
        void HandleDashInput()
        {
            if (!Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_RIGHT)) return;

            bool groundedNow = grounded;

            // Dashes cancel the attack
            if (isAttacking)
            {
                CancelAttackState();
            }

            if (groundedNow)
            {
                if (!isGroundDashing && Time.time >= groundDashCooldownUntil) BeginGroundDash();
            }
            else
            {
                if (!isAirDashing && Time.time >= airDashCooldownUntil) BeginAirDash();
            }
        }

        Vector3 ComputeFlatDashDir(bool useMoveDir)
        {
            Vector2 input = GetAxis();
            bool hasInput = input.SquareMagnitude() > 0.0001f;
            if (hasInput) input.Normalize();

            if (!useMoveDir || !hasInput)
            {
                Vector3 dir = (dashDefaultBackwards ? -transform.Forward: transform.Forward);
                dir.y = 0f;
                return dir.Normalize();
            }

            if (camRig != null)
            {
                Vector3 camFwd = camRig.GetFlatAimDirection(transform);
                Vector3 camRight = Vector3.Cross(Vector3.Up, camFwd).Normalize();
                Vector3 moveDirInput = camFwd * input.y + camRight * input.x;
                moveDirInput.y = 0f;
                return moveDirInput.SquareMagnitude() > 0.0001f ? moveDirInput.Normalize()
                                                           : ((dashDefaultBackwards ? -transform.Forward: transform.Forward));
            }
            else
            {
                Vector3 moveDirInput = transform.Forward * input.y + transform.Right* input.x;
                moveDirInput.y = 0f;
                return moveDirInput.SquareMagnitude() > 0.0001f ? moveDirInput.Normalize()
                                                           : ((dashDefaultBackwards ? -transform.Forward : transform.Forward));
            }
        }

        Vector3 AddUpwardAngle(Vector3 flatDir, float angleDeg)
        {
            flatDir.y = 0f;
            if (flatDir.SquareMagnitude() < 0.0001f) flatDir = transform.Forward;
            flatDir.Normalize();
            float rad = (float)(angleDeg * ((Math.PI * 2) / 360));
            Vector3 tilted = (flatDir * Math.Cos(rad)) + (Vector3.Up * Math.Sin(rad));
            return tilted.Normalize();
        }

        void BeginGroundDash()
        {
            isGroundDashing = true;
            isAirDashing = false;

            Vector3 flat = ComputeFlatDashDir(dashUsesMoveDirection);
            dashDir = AddUpwardAngle(flat, groundDashUpAngleDeg);
            dashTimer = Math.Max(0.0001f, dashStartDuration);

            //Vector3 flatFacing = transform.Forward; flatFacing.y = 0f; flatFacing.Normalize();
            //Vector3 flatDash = dashDir; flatDash.y = 0f; flatDash.Normalize();
            //float dot = Vector3.Dot(flatDash, flatFacing);
            //bool isBackDash = dot < backDashDotThreshold;

            //if (useBackdashAnimation && isBackDash)
            //{
            //    animator.ResetTrigger(backDashTrigger);
            //    animator.SetTrigger(backDashTrigger);
            //}
            //else
            //{
            //    animator.ResetTrigger(fwdDashTrigger);
            //    animator.SetTrigger(fwdDashTrigger);
            //}

            //AudioManager.instance.PlaySFX("Dash");          //Play Dash SFX
            //StartCoroutine(SpawnVFX(dashVFX[0]));           //Dash VFX
            //if (dashEffectsCoroutine != null)
            //{
            //    StopCoroutine(dashEffectsCoroutine);
            //    dashEffectsCoroutine = null;
            //}
            //dashEffectsCoroutine = StartCoroutine(DashEffects());
            //// Someone forgot to start their coroutines :(
            //camRig.DashFOVKick(8f, 0.08f, 0.04f, 0.10f);    //Camera FOV
            //camRig.Shake(10f, 0.12f);                       //Camera Shake
        }

        void BeginAirDash()
        {
            isAirDashing = true;
            isGroundDashing = false;

            Vector3 flat = ComputeFlatDashDir(dashUsesMoveDirection);
            dashDir = AddUpwardAngle(flat, airDashUpAngleDeg);

            dashTimer = Math.Max(0.0001f, airDashDuration);
            velocity.y = 0f;

            animator.SetBool("Grounded", false);

            Vector3 flatFacing = transform.Forward; flatFacing.y = 0f; flatFacing.Normalize();
            Vector3 flatDash = dashDir; flatDash.y = 0f; flatDash.Normalize();
            float dot = Vector3.Dot(flatDash, flatFacing);
            bool isBackDash = dot < backDashDotThreshold;

            //if (useBackdashAnimation && isBackDash)
            //{
            //    animator.ResetTrigger(backDashTrigger);
            //    animator.SetTrigger(backDashTrigger);
            //}
            //else
            //{
            //    animator.ResetTrigger(airDashTrigger);
            //    animator.SetTrigger(airDashTrigger);
            //}

            //AudioManager.instance.PlaySFX("Dash");          //Play Dash SFX
            //StartCoroutine(SpawnVFX(dashVFX[0]));           //Dash VFX
            //if (dashEffectsCoroutine != null)
            //{
            //    StopCoroutine(dashEffectsCoroutine);
            //    dashEffectsCoroutine = null;
            //}
            //dashEffectsCoroutine = StartCoroutine(DashEffects());

            //camRig.DashFOVKick(8f, 0.08f, 0.04f, 0.10f);    //Camera FOV
            //camRig.Shake(10f, 0.12f);                       //Camera Shake
        }

        void UpdateDash()
        {
            if (isGroundDashing || isAirDashing)
            {
                dashTimer -= Time.deltaTime;
                if (dashTimer <= 0f)
                {
                    if (isGroundDashing) EndGroundDash();
                    else EndAirDash();
                }
            }
        }

        void EndGroundDash()
        {
            isGroundDashing = false;
            groundDashCooldownUntil = Time.time + dashCooldown;
        }

        void EndAirDash()
        {
            isAirDashing = false;
            airDashCooldownUntil = Time.time + airDashCooldown;
        }

        //private IEnumerator DashEffects()
        //{
        //    lensDistortion.intensity.value = -0.8f;
        //    float timer = 0f;
        //    while (timer <= Mathf.Max(lensDistortionDuration, playerGlowDuration))
        //    {
        //        timer += Time.deltaTime;
        //        lensDistortion.intensity.value = Mathf.Lerp(-0.5f, 0f, timer / lensDistortionDuration);

        //        foreach (MeshRenderer mr in playerMat)
        //        {
        //            mr.material.SetFloat("_GlowAmount", Mathf.Lerp(1, 0, timer / playerGlowDuration));
        //        }

        //        yield return null;
        //    }
        //}

        #endregion

        // -------------------- Attack Input & Update ------------------------------------------------------------------------------------------
        #region Attack Inputs

        private void InitializeHitboxes()
        {
            //basicHB_1.HitBoxListeners += Attack_1_Damage;
            //basicHB_2.HitBoxListeners += Attack_2_Damage;
            //basicHB_3.HitBoxListeners += Attack_3_Damage;
            //plungeHB.HitBoxListeners += Plunge_Damage;
        }

        //public void Attack_1_Damage(GameObject entityToDamage)
        //{
        //    if (entityToDamage.TryGetComponent<EnemyBehaviour>(out EnemyBehaviour objectToDamage))
        //    {
        //        objectToDamage.TakeDamage(attack1_Damage, gameObject);
        //    }
        //}

        //public void Attack_2_Damage(GameObject entityToDamage)
        //{
        //    if (entityToDamage.TryGetComponent<EnemyBehaviour>(out EnemyBehaviour objectToDamage))
        //    {
        //        objectToDamage.TakeDamage(attack2_Damage, gameObject);
        //    }
        //}
        //public void Attack_3_Damage(GameObject entityToDamage)
        //{
        //    if (entityToDamage.TryGetComponent<EnemyBehaviour>(out EnemyBehaviour objectToDamage))
        //    {
        //        objectToDamage.TakeDamage(attack3_Damage, gameObject);
        //    }
        //}
        //public void Plunge_Damage(GameObject entityToDamage)
        //{
        //    if (entityToDamage.TryGetComponent<EnemyBehaviour>(out EnemyBehaviour objectToDamage))
        //    {
        //        objectToDamage.TakeDamage((int)((float)plunge_Damage * (1f + _plungemodifierCount)), gameObject);
        //    }
        //}



        void HandleAttackInput()
        {
            if (!grounded && !isAttacking && !isPlunging) // Plunge: only if airborne and not already attacking

            {
                if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT))
                {
                    BeginPlunge();
                    return; // don't go into normal combo logic
                }
            }

            if (isGroundDashing || isAirDashing) return;

            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT))
            {
                if (!isAttacking)
                {
                    BeginAttack(1);
                }
                else
                {
                    if (IsInChainWindow()) // If inside chain window: queue next, and snap facing if WASD is held.
                    {
                        queuedNext = true;

                        Vector3 desiredFacing;
                        bool hasMoveInput = TryGetCameraRelativeMove(out desiredFacing);

                        if (hasMoveInput)
                        {
                            desiredFacing.y = 0f;
                            if (desiredFacing.SquareMagnitude() > 0.0001f)
                            {
                                transform.RotationQuat = Quaternion.LookRotation(desiredFacing, Vector3.Up);
                                queuedFacingOverride = true;
                                queuedFacing = desiredFacing.Normalize();
                            }
                        }
                        else
                        {
                            queuedFacingOverride = true;
                            Vector3 f = transform.Forward; f.y = 0f;
                            queuedFacing = (f.SquareMagnitude() > 0.0001f) ? f.Normalize(): Vector3.Forward;
                        }
                    }
                }
            }
        }

        void BeginAttack(int index)
        {
            canPlaySlimeHitSFX = true;
            isAttacking = true;
            attackIndex = index;
            attackTimer = 0f;

            Vector3 facingForThisAttack;    // Determine facing for this attack (consume any queued override decided during chain press)

            if (queuedFacingOverride && queuedFacing.SquareMagnitude() > 0.0001f)
            {
                facingForThisAttack = queuedFacing.Normalize();
                transform.RotationQuat = Quaternion.LookRotation(facingForThisAttack, Vector3.Up);
            }
            else
            {
                Vector3 f = transform.Forward; f.y = 0f;
                facingForThisAttack = (f.SquareMagnitude() > 0.0001f) ? f.Normalize() : Vector3.Forward;
            }

            queuedFacingOverride = false; // consume
            queuedNext = false;           // reset for this stage

            if (index == 1 || index == 2)   // Setup movement burst for each attack
            {
                float dist = (index == 1) ? atk1LungeDistance : atk2LungeDistance;
                float dur = (index == 1) ? atk1LungeDuration : atk2LungeDuration;
                float del = (index == 1) ? atk1LungeDelay : atk2LungeDelay;
                PrepareLunge(dist, dur, del, facingForThisAttack);
            }
            else // Attack 3
            {
                isLunging = false;  // not used in A3
                lungeStarted = false;

                PrepareAtk3Arc(facingForThisAttack);
            }


            //if (index == 1)
            //{
            //    animator.ResetTrigger(atk1Trigger);
            //    animator.SetTrigger(atk1Trigger);
            //    StartCoroutine(SpawnVFX(slashVFX[0]));
            //    AudioManager.instance.PlaySFX("A1");
            //}
            //else if (index == 2)
            //{
            //    animator.ResetTrigger(atk2Trigger);
            //    animator.SetTrigger(atk2Trigger);
            //    StartCoroutine(SpawnVFX(slashVFX[1]));
            //    AudioManager.instance.PlaySFX("A2");
            //}
            //else
            //{
            //    animator.ResetTrigger(atk3Trigger);
            //    animator.SetTrigger(atk3Trigger);
            //    StartCoroutine(SpawnVFX(slashVFX[2]));
            //    AudioManager.instance.PlaySFX("A3");
            //}
            //animator.SetBool("IsAttacking", true);
        }

        void UpdateAttack()
        {
            if (!isAttacking) return;

            attackTimer += Time.deltaTime;

            // Active per-attack motion
            if (attackIndex == 1 || attackIndex == 2)
                UpdateLunge();
            else if (attackIndex == 3)
            {
                UpdateAtk3Arc();
            }

            float dur = GetCurrentAttackDuration();

            if (attackTimer >= dur)
            {
                if (queuedNext)
                {
                    if (attackIndex == 1) BeginAttack(2);
                    else if (attackIndex == 2) BeginAttack(3);
                    else EndAttackToIdle();
                }
                else
                {
                    if (attackIndex == 1)
                    {
                        //animator.ResetTrigger(atkToIdle1Trigger);
                        //animator.SetTrigger(atkToIdle1Trigger);
                        EndAttackState();
                    }
                    else if (attackIndex == 2)
                    {
                        //animator.ResetTrigger(atkToIdle2Trigger);
                        //animator.SetTrigger(atkToIdle2Trigger);
                        EndAttackState();
                    }
                    else
                    {
                        EndAttackToIdle();
                    }
                }
            }
        }

        void EndAttackToIdle()
        {
            EndAttackState();
        }

        void EndAttackState()
        {
            isAttacking = false;
            attackIndex = 0;
            attackTimer = 0f;
            queuedNext = false;
            queuedFacingOverride = false;

            // Clear movement bursts
            isLunging = false;
            lungeStarted = false;

            atk3ArcActive = false;
            atk3ImpulseFired = false;
            atk3HorizVel = Vector3.Zero;

            //animator.SetBool("IsAttacking", false);
        }

        void CancelAttackState()
        {
            EndAttackState();
        }

        bool IsInChainWindow()
        {
            float dur = GetCurrentAttackDuration();
            float lead = GetCurrentChainLead();
            return isAttacking && attackTimer >= (dur - lead) && attackTimer <= dur;
        }

        float GetCurrentAttackDuration() //Used to determine snappiness of next input
        {
            switch (attackIndex)
            {
                case 1: return atk1Duration;
                case 2: return atk2Duration;
                case 3: return atk3Duration;
                default: return 0f;
            }
        }

        float GetCurrentChainLead()
        {
            switch (attackIndex)
            {
                case 1: return atk1ChainLead;
                case 2: return atk2ChainLead;
                case 3: return atk3ChainLead;
                default: return 0f;
            }
        }

        #endregion

        // -------------------- Lunge ------------------------------------------------------------------------------------------
        #region Lunge

        void PrepareLunge(float distance, float duration, float delay, Vector3 facingOverride)
        {
            isLunging = true;
            lungeStarted = false;
            lungeTimer = 0f;
            lungeDuration = Math.Max(0.0001f, duration);
            lungeDelay = Math.Max(0f, delay);

            Vector3 f = facingOverride; f.y = 0f;
            if (f.SquareMagnitude() < 0.0001f)
            {
                f = transform.Forward; f.y = 0f;
            }
            lungeDir = f.Normalize();
            lungeSpeed = distance / lungeDuration;
        }

        void UpdateLunge()
        {
            if (!isLunging) return;

            lungeTimer += Time.deltaTime;

            if (!lungeStarted)
            {
                if (lungeTimer >= lungeDelay)
                {
                    lungeStarted = true;
                }
                else
                {
                    return;
                }
            }

            if (lungeStarted && lungeTimer <= (lungeDelay + lungeDuration))
            {
                Vector3 lungeVel = lungeDir * lungeSpeed;
                Vector3 finalMove = new Vector3(lungeVel.x, 0f, lungeVel.z) * Time.deltaTime;
                transform.Position += finalMove;
            }
            else
            {
                isLunging = false;
            }
        }

        #endregion

        // -------------------- Attack 3 Jump ------------------------------------------------------------------------------------------
        #region Attack 3

        void PrepareAtk3Arc(Vector3 facingOverride)
        {
            atk3ArcActive = true;
            atk3ArcTimer = 0f;
            atk3ImpulseFired = false;

            atk3ArcDir = facingOverride; atk3ArcDir.y = 0f;
            if (atk3ArcDir.SquareMagnitude() < 0.0001f) atk3ArcDir = transform.Forward;
            atk3ArcDir.Normalize();

            atk3HorizVel = Vector3.Zero;
            atk3UpwardEndTime = 0f;
        }

        void UpdateAtk3Arc()
        {
            if (!atk3ArcActive) return;

            atk3ArcTimer += Time.deltaTime;

            // Apply the impulse once after the delay
            if (!atk3ImpulseFired && atk3ArcTimer >= atk3ImpulseDelay)
            {
                atk3ImpulseFired = true;

                // Upward
                velocity.y = atk3UpwardSpeed;
                atk3UpwardEndTime = atk3ArcTimer + atk3UpwardDuration;

                // Forward
                atk3HorizVel = atk3ArcDir * atk3ForwardSpeed;
            }

            if (atk3ImpulseFired)
            {
                // Apply forward while inside forward duration
                if (atk3ArcTimer <= atk3ImpulseDelay + atk3ForwardDuration)
                {
                    transform.Position += atk3HorizVel * Time.deltaTime;
                }

                // Cancel upward velocity after upward duration ends
                if (atk3ArcTimer >= atk3UpwardEndTime)
                {
                    if (velocity.y > 0f) velocity.y = 0f;
                }
            }

            // When both durations have passed, we can end arc state
            if (atk3ArcTimer > atk3ImpulseDelay + Math.Max(atk3ForwardDuration, atk3UpwardDuration))
            {
                atk3ArcActive = false;
                atk3HorizVel = Vector3.Zero;
            }
        }

        #endregion

        // -------------------- Plunge Attack ------------------------------------------------------------------------------------------
        #region Plunge

        void BeginPlunge()
        {
            //AudioManager.instance.PlaySFX("Plunge");
            isPlunging = true;
            plungeTimer = 0f;
            plungeImpulseStarted = false;

            //Eze's Code Start
            _plungemodifierCount = 0f;

            _plungeStartPoint = new Vector3(this.transform.Position.x, this.transform.Position.y, this.transform.Position.z);

            //Eze's Code End

            velocity = Vector3.Zero;  // stop dead

            plungeDir = transform.Forward; plungeDir.y = 0f;
            if (plungeDir.SquareMagnitude() < 0.001f) plungeDir = Vector3.Forward;
            plungeDir.Normalize();

            //animator.ResetTrigger(plungeTrigger);
            //animator.SetTrigger(plungeTrigger);
            //animator.SetBool("IsAttacking", true);
        }

        void UpdatePlunge()
        {
            if (!isPlunging) return;

            plungeTimer += Time.deltaTime;

            //Eze's Code Start

            float heightTraveled = _plungeStartPoint.y - this.transform.Position.y;



            if (heightTraveled >= heightThreshold)
            {
                _plungemodifierCount = (heightTraveled - heightThreshold) * heightModifierPerOneExtra;
            }

            //Eze's Code End

            if (!plungeImpulseStarted && plungeTimer >= plungeDelay)
                plungeImpulseStarted = true;

            if (plungeImpulseStarted && plungeTimer <= plungeDelay + plungeDuration)
            {
                Vector3 slamVel = (plungeDir * plungeForwardSpeed) + (Vector3.Down * plungeDownwardSpeed);
                transform.Position += slamVel * Time.deltaTime;
            }

            if ((plungeImpulseStarted && plungeTimer >= plungeDelay + plungeDuration) || grounded)
                EndPlunge();
        }


        void EndPlunge()
        {
            isPlunging = false;
            plungeTimer = 0f;
            plungeImpulseStarted = false;
            //StartCoroutine(SpawnVFX(slashVFX[3]));

            animator.SetBool("IsAttacking", false);
        }

        #endregion

        // -------------------- Devices ------------------------------------------------------------------------------------------
        #region Devices

        //private void HandleDeviceInput()
        //{
        //    if (Input.IsKeyDown(Keys.KEY_R))
        //    {
        //        TeleportBack();
        //    }

        //}
        //void TeleportBack()
        //{
        //    if (!_teleportCoolingDown)
        //    {
        //        ignoreMovementThisFrame = true;

        //        this.ForceWarp(PayloadBehaviour.instance.TeleportPoint.position);
        //        //this.transform.position = PayloadBehaviour.instance.TeleportPoint.position;
        //        //this.transform.rotation = Quaternion.Euler(new Vector3(0, PayloadBehaviour.instance.TeleportPoint.rotation.eulerAngles.y , 0));

        //        StartCoroutine(TeleportCooldown());
        //    }
        //    else
        //    {

        //    }
        //}

        //IEnumerator TeleportCooldown()
        //{
        //    float count = 0f;
        //    _teleportCoolingDown = true;

        //    HUDController.Instance.ToggleTeleport(false);

        //    while (_teleportCoolingDown)
        //    {
        //        count += Time.fixedDeltaTime;
        //        if (count >= teleportCooldown)
        //        {
        //            _teleportCoolingDown = false;
        //        }
        //        else
        //        {
        //            //HUDController.Instance.SetTauntSlider(count / tauntCooldown);
        //        }
        //        yield return new WaitForSeconds(Time.fixedDeltaTime);
        //    }

        //    //HUDController.Instance.SetTauntSlider(1f);
        //    HUDController.Instance.ToggleTeleport(true);
        //}

        #endregion

        // -------------------- Ultimate ------

        #region Ultimate


        bool _ultiOnCooldown = true;


        //public void TryToUlt()
        //{
        //    if (!_ultiOnCooldown)
        //    {
        //        ultimate.SetActive(true);

        //        StartCoroutine(UltiCooldown());
        //    }
        //}

        //IEnumerator UltiCooldown()
        //{
        //    float count = 0f;
        //    _ultiOnCooldown = true;

        //    //HUDController.Instance.ToggleTaunt(false);

        //    HUDController.Instance.UltimateUsed();

        //    while (_ultiOnCooldown)
        //    {
        //        count += Time.fixedDeltaTime;
        //        if (count >= ultiCooldown)
        //        {
        //            _ultiOnCooldown = false;
        //        }
        //        else
        //        {
        //            //HUDController.Instance.SetTauntSlider(count / tauntCooldown);
        //            HUDController.Instance.SetUltimateSlider(count / ultiCooldown);
        //        }
        //        yield return new WaitForSeconds(Time.fixedDeltaTime);
        //    }

        //    //HUDController.Instance.SetTauntSlider(1f);
        //    //HUDController.Instance.ToggleTaunt(true);
        //    HUDController.Instance.UltimateReady();

        //    yield break;

        //}

        #endregion
        // -------------------- Helpers ------------------------------------------------------------------------------------------
        bool TryGetCameraRelativeMove(out Vector3 dir)
        {
            Vector2 input = GetAxis();

            if (input.SquareMagnitude() > 0.0001f)
            {
                input.Normalize();
                if (camRig != null)
                {
                    Vector3 camFwd = camRig.GetFlatAimDirection(transform);
                    Vector3 camRight = Vector3.Cross(Vector3.Up, camFwd).Normalize();
                    Vector3 moveDirInput = camFwd * input.y + camRight * input.x;
                    moveDirInput.y = 0f;
                    if (moveDirInput.SquareMagnitude() > 0.0001f)
                    {
                        dir = moveDirInput.Normalize();
                        return true;
                    }
                }
                else
                {
                    Vector3 moveDirInput = transform.Forward * input.y + transform.Right* input.x;
                    moveDirInput.y = 0f;
                    if (moveDirInput.SquareMagnitude()> 0.0001f)
                    {
                        dir = moveDirInput.Normalize();
                        return true;
                    }
                }
            }
            dir = Vector3.Zero;
            return false;
        }

        //private IEnumerator SpawnVFX(VFX vfxToSpawn, bool parent = false)
        //{
        //    yield return new WaitForSeconds(vfxToSpawn.delay);
        //    var go = Instantiate(vfxToSpawn.vfxPrefab, VFXZeroPoint.position, VFXZeroPoint.rotation, parent ? transform : null);
        //    Destroy(go, 5f);
        //}

        // -------------------- Ember ------------------------------------------------------------------------------------------
        #region Ember
        /* //=== EZE'S GAS REMOVE EDIT ===
        public bool AddGas(int amount)
        {
            if (currentGas >= maxGas)
            {
                currentGas = maxGas;
                return false;
            }
            else
            {
                currentGas += amount;
                HUDController.Instance.SetPlayerEmber((float)currentGas / (float)maxGas);
                return true;
            }
            //Add the rest of effects that relies on this
        }

        public bool RemoveGas(int amount)
        {
            if (currentGas <= 0)
            {
                currentGas = 0;
                return false;
            }
            else
            {
                currentGas -= amount;
                HUDController.Instance.SetPlayerEmber((float)currentGas / (float)maxGas);
                return true;
            }
        }

        */ //=== EZE'S GAS REMOVE EDIT ===
        #endregion

        // -------------------- Item ------------------------------------------------------------------------------------------
        #region Item

        private int itemsCollected = 0;
        public int ItemsCollected
        { get { return itemsCollected; } }

        public void OnCollect()
        {
            itemsCollected++;
            // Add any additional logic for collecting items, such as updating UI or playing sound effects
        }
        public int DropOffItems()
        {
            int amount = itemsCollected; // Returns the amount of items dropped off
            itemsCollected -= itemsCollected;
            return amount;
            // Add any additional logic for dropping off items, such as updating UI or playing sound effects
        }

        protected override void OnHeal()
        {
        }

        protected override void OnDamaged(GameObject source)
        {
        }

        public override void OnDeath()
        {
        }

        #endregion

        // -------------------- Entity Overrides ------------------------------------------------------------------------------------------
        #region Entity Overrides
        //public override void TakeDamage(int amount, GameObject source = null)
        //{
        //    if (UltimateAnimationEventForwarder.IsUlting) return;
        //    base.TakeDamage(amount, source);
        //}
        //protected override void OnHeal()
        //{
        //    //throw new System.NotImplementedException();
        //    HUDController.Instance.SetHealth((float)Health / (float)MaxHealth);
        //}

        //protected override void OnDamaged(GameObject source)
        //{
        //    HUDController.Instance.SetHealth((float)Health / (float)MaxHealth);
        //    StopRegen();
        //}

        //public override void OnDeath()
        //{
        //    if (dead != true)
        //    {
        //        LevelDirector.Instance.LoseGame();
        //        dead = true;
        //    }
        //}

        #endregion
    }
}
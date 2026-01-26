using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;


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
        public float rotationSpeed = 12f;
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
        private float verticalVelocity = 0f;
        public float terminalVelocity = -50f;

        private Vector3 velocity;
        private bool wasGrounded;
        private float lastGroundedTime;
        private float lastJumpPressedTime;
        private bool jumpRequested;
        private bool jumpImpulseApplied;
        private float jumpApplyAtTime;
        private bool doubleJumpAvailable;

        // Jump and Gravity
        public float jumpHeight = 1.4f;
        public float gravity = -9.81f;
        public float groundedGravity = -2f;
        public float coyoteTime = 0.08f;
        public float jumpBuffer = 0.10f;
        private float jumpDelay = 0.00f;
        private float lastAirTime = float.NegativeInfinity;
        private float lastLandTime = float.NegativeInfinity;
        public float minAirTimeForLanding = 0.06f;
        public float landCooldown = 0.08f;           // prevents immediate retriggering

        private bool enableDoubleJump = true;
        public float doubleJumpHeight = 1.2f;
        private bool resetYOnDoubleJump = true;

        // Dash Variables
        private bool isGroundDashing = false;
        private bool isAirDashing = false;
        private float dashTimer = 0f;
        private float groundDashCooldownUntil = 0f;
        private float airDashCooldownUntil = 0f;
        private Vector3 dashDir = Vector3.Zero;

        public float dashDistance = 6f;
        public float dashStartDuration = 0.25f;
        public float dashCooldown = 0.6f;
        public bool dashUsesMoveDirection = true;
        public float groundDashUpAngleDeg = 0f;

        private bool useBackdashAnimation = true;
        public float backDashDotThreshold = 0f;
        private bool dashDefaultBackwards = true;
        private bool rotateToDashDirection = false;

        public float airDashDistance = 5f;
        public float airDashDuration = 0.25f;
        public float airDashCooldown = 0.8f;
        public float airDashUpAngleDeg = 15f;

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

        // =============== Internal variables =============== 
        public bool canInput = false;
        public bool canMove = false;
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
            if (canInput)
            {
                HandleInput();
                HandleDashInput();
            }
            UpdateDash();
            if (canMove) HandleMovement();
            HandleJump();
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

            if (input.SquareMagnitude() > 1f) input = input.Normalize();

            if (input != Vector3.Zero)
            {
                if (animator.SafeToChange("Walk"))
                    animator.SetBool("Walk", true);
            }
            else
            {
                if (animator.SafeToChange("Idle"))
                    animator.SetBool("Idle", true);
            }

            //if (Input.IsKeyPressed(Keys.KEY_SPACEBAR)) TryJump();
            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
        }
        #region New Movement
        private void HandleMovement()
        {
            Vector3 camForward = new Vector3();
            if (camera != null)
            {
                camForward = camera.transform.RotationQuat * Vector3.Forward; // Get camera forward direction
                camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
                camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera
            }

            Vector3 camRight = Vector3.Cross(Vector3.Up, camForward).Normalize();
            Vector3 moveDirInput = camForward * input.z + camRight * input.x;

            float rawPlanarSpeed = moveDirInput.Magnitude() * movementSpeed;
            bool walkingNow = !isAttacking && !isGroundDashing && !isAirDashing && grounded && rawPlanarSpeed > 0.1f;
            //SliceLog.Log("GroundDashing: " + isGroundDashing.ToString() + " AirDashing: " + isAirDashing.ToString() + " isAttacking: " + isAttacking.ToString() + " isPlunging: " + isPlunging.ToString());
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

                if (animator != null)
                {
                    if (isGroundDashing)
                    {
                        //animator.SetBool("DashStart", true);
                    }
                    else if (isAirDashing)
                    {

                    }
                    //
                }
            }
            else if (isAttacking || isPlunging)
            {
                if (isPlunging && !plungeImpulseStarted)
                {
                    // apply impulse
                    plungeImpulseStarted = true;
                    // ill jus copy the jump but apply it downwards
                    float jumpSpeed = (float)Math.Sqrt(doubleJumpHeight * gravity);
                    velocity.y = -1.0f; // physics crashes when I try to do any number thats too big for some reason
                }
                // if it grounds when plunging
                if (isPlunging && grounded)
                {
                    isPlunging = false;
                    plungeImpulseStarted = false;
                    // transition to plunge land
                    if (animator != null)
                    {
                        animator.SetBool("PlungeLand", true);

                        // check if there is input
                        if (input != Vector3.Zero)
                            animator.SetBool("PlungeToWalk", true);
                        else
                            animator.SetBool("PlungeToIdle", true);


                        // if there is then transition to plunge walk
                        // else transition to plunge idle?
                    }
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

                Vector3 horizontal = moveDirInput * movementSpeed;
                Vector3 finalMove = new Vector3(horizontal.x, velocity.y, horizontal.z);
                transform.Position += finalMove * Time.deltaTime;
            }

            //if (animator)
            //{
            //    animator.SetFloat("Speed", (isAttacking ? 0f : rawPlanarSpeed));
            //    //animator.SetFloat("YVel", velocity.y);
            //    //animator.SetBool("IsDashing", isGroundDashing || isAirDashing);
            //    animator.SetBool("IsAttacking", isAttacking);
            //}


            //if (walkAudioSource != null)
            //{
            //    if (walkingNow)
            //    {
            //        if (!walkAudioSource.isPlaying)
            //            walkAudioSource.Play();
            //    }
            //    else
            //    {
            //        if (walkAudioSource.isPlaying)
            //            walkAudioSource.Stop();
            //    }
            //}
        }

        // -------------------- Jump ------------------------------------------------------------------------------------------
        void HandleJump()
        {
            if (grounded) lastGroundedTime = Time.time;

            if (wasGrounded && !grounded)
            {
                lastAirTime = Time.time; // mark when airborne
                if (animator != null)
                {
                    if (animator.SafeToChange("JumpLoop"))
                        animator.SetBool("JumpLoop", true);
                }
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
                    //if (animator) animator.SetBool("Grounded", true);
                }


                if (animator != null)
                {
                    if (animator.SafeToChange("Land"))
                        animator.SetBool("Land", true);
                }

                doubleJumpAvailable = enableDoubleJump;
                jumpRequested = false;
                jumpImpulseApplied = false;
            }

            // Jump input disabled during attacks
            if (!isAttacking && Input.IsKeyPressed(Keys.KEY_SPACEBAR))
                lastJumpPressedTime = Time.time;

            bool canCoyote = (Time.time - lastGroundedTime) <= coyoteTime;
            bool bufferedJump = (Time.time - lastJumpPressedTime) <= jumpBuffer;

            if (!grounded && !canCoyote)
            {
                if (!isAttacking && enableDoubleJump && doubleJumpAvailable && Input.IsKeyPressed(Keys.KEY_SPACEBAR))
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

                    if (animator != null)
                    {
                        //animator.SetBool("JumpLoop", true);
                    }

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
            else if (isPlunging && !plungeImpulseStarted)
            {
                velocity.y = 0f;
            }
            else if (grounded && velocity.y <= 0)
            {
                // Hard lock to ground
                velocity.y = 0f;
            }
            else
            {
                // Airborne → apply gravity
                velocity.y += gravity * Time.deltaTime;
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

            if (animator != null)
            {
                animator.SetBool("AirDashStart", true);
            }

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
            if (!Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_RIGHT)) return;

            // Dashes cancel the attack
            if (isAttacking)
            {
                EndAttackState();
            }

            if (grounded)
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
            //bool hasInput = input.SquareMagnitude() > 0.0001f;
            //if (hasInput) input = input.Normalize();

            //if (!useMoveDir || !hasInput)
            //{
            //    Vector3 dir = (dashDefaultBackwards ? -transform.Forward : transform.Forward);
            //    dir.y = 0f;
            //    return dir.Normalize();
            //}

            //if (camera != null)
            //{
            //    Vector3 camForward = new Vector3();
            //    camForward = camera.transform.RotationQuat * Vector3.Forward; // Get camera forward direction
            //    camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
            //    camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera

            //    return input.SquareMagnitude() > 0.0001f ? camForward
            //                                               : ((dashDefaultBackwards ? -transform.Forward : transform.Forward));
            //}
            //else
            //{
            //    Vector3 moveDirInput = transform.Forward * input.z + transform.Right * input.x;
            //    moveDirInput.y = 0f;
            //    return moveDirInput.SquareMagnitude() > 0.0001f ? moveDirInput.Normalize()
            //                                               : ((dashDefaultBackwards ? -transform.Forward : transform.Forward));
            //}
            bool hasInput = input.SquareMagnitude() > 0.0001f;
            if (hasInput)
                input = input.Normalize();

            // --- NO INPUT OR FORCED FORWARD ---
            if (!useMoveDir || !hasInput)
            {
                Vector3 forward = transform.Forward;
                forward.y = 0f;
                return (dashDefaultBackwards ? -forward : forward).Normalize();
            }

            // --- CAMERA-RELATIVE DASH ---
            if (camera != null)
            {
                Vector3 camForward = camera.transform.RotationQuat * Vector3.Forward;
                camForward.y = 0f;
                camForward = camForward.Normalize();

                Vector3 camRight = Vector3.Cross(Vector3.Up, camForward);

                Vector3 dashDir =
                    camForward * input.z +
                    camRight * input.x;

                dashDir.y = 0f;
                return dashDir.Normalize();
            }

            // --- TRANSFORM-RELATIVE FALLBACK ---
            Vector3 moveDir =
                transform.Forward * input.z +
                transform.Right * input.x;

            moveDir.y = 0f;
            return moveDir.Normalize();
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

            //if (animator)
            {
                Vector3 flatFacing = transform.Forward;
                flatFacing.y = 0f;
                flatFacing.Normalize();
                Vector3 flatDash = transform.Forward;
                flatDash.y = 0f;
                flatDash.Normalize();
                float dot = Vector3.Dot(flatDash, flatFacing);
                bool isBackDash = dot < backDashDotThreshold;

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
            }

            if (animator != null)
            {
                SliceLog.Log("Dash???");
                animator.SetBool("DashStart", true);
            }

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

            //if (animator)
            {
                //animator.SetBool("Grounded", false);

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
            }

            if (animator != null)
            {
                animator.SetBool("AirDashStart", true);
            }

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

            //if (animator) animator.SetBool("IsAttacking", false);
        }
        #endregion
        #region Movement
        //private void OldHandleMovement()
        //{
        //    Transform camTransform;

        //    if (camera == null)
        //    {
        //        return;
        //    }
        //    else
        //    {
        //        camTransform = camera.transform;
        //    }

        //    Vector3 camForward = camTransform.RotationQuat * Vector3.Forward; // Get camera forward direction
        //    camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
        //    camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera

        //    Vector3 moveDir = camForward * input.z + Vector3.Cross(Vector3.Up, camForward).Normalize() * input.x;

        //    if (isDashing) return;

        //    if (grounded)
        //    {
        //        // Reset vertical velocity when grounded
        //        if (verticalVelocity < 0f)
        //            verticalVelocity = 0f;
        //    }
        //    else
        //    {
        //        // Apply gravity
        //        verticalVelocity += gravity * Time.deltaTime;

        //        // Optional terminal velocity clamp
        //        if (verticalVelocity < terminalVelocity)
        //            verticalVelocity = terminalVelocity;
        //    }

        //    // Handle regular movement
        //    Vector3 horizontalMovement = moveDir * moveSpeed;

        //    if (moveDir.LengthSquared() > 0.01f)
        //    {
        //        playerModel.GetComponent<Transform>().RotationQuat = Quaternion.LookRotation(moveDir, Vector3.Up);
        //    }

        //    if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_RIGHT))
        //    {
        //        if (!isDashing) StartCoroutine(Dash(moveDir));
        //    }

        //    transform.Position += (horizontalMovement + new Vector3(0f, verticalVelocity, 0f)) * Time.deltaTime;

        //    //Console.WriteLine(animator.GetCurrAnimName());
        //}
        //private void HandlePhysicsMovement()
        //{
        //    Transform camTransform;

        //    if (camera == null)
        //    {
        //        return;
        //    }
        //    else
        //    {
        //        camTransform = camera.transform;
        //    }

        //    Vector3 camForward = camTransform.RotationQuat * Vector3.Forward; // Get camera forward direction
        //    camForward.y = 0f; // Ignore vertical axis so it'll move parallel to ground
        //    camForward = camForward.Normalize(); // Get the normal vector which is the direction of the camera
        //    Vector3 moveDir = camForward * input.z + Vector3.Cross(Vector3.Up, camForward).Normalize() * input.x;

        //    if (isDashing) return;

        //    // Handle regular movement
        //    Vector3 movement = moveDir * moveSpeed;
        //    bool falling = rb.Velocity.y < fallVelocityThreshold && !grounded;
        //    if (falling) rb.AddForce(Vector3.Down * fallForce, ForceMode.Impulse);

        //    rb.Velocity = new Vector3(movement.x, rb.Velocity.y, movement.z);

        //    if (moveDir.LengthSquared() > 0.01f)
        //    {
        //        playerModel.GetComponent<Transform>().RotationQuat = Quaternion.LookRotation(moveDir, Vector3.Up);
        //    }

        //    if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_RIGHT))
        //    {
        //        if (!isDashing) StartCoroutine(Dash(moveDir));
        //    }


        //    //Console.WriteLine(animator.GetCurrAnimName());
        //}
        //private void TryJump()
        //{
        //    if (CanJump())
        //    {

        //        rb.Velocity = new Vector3(rb.Velocity.x, 0f, rb.Velocity.z); // Reset vertical velocity before applying jump force
        //        jumpCounter++;

        //        if (jumpCounter == 1)
        //        {
        //            if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0 ||
        //                    String.Compare(animator.GetCurrAnimName(), "Walk") == 0 ||
        //                    String.Compare(animator.GetCurrAnimName(), "AttackToIdle1") == 0 ||
        //                    String.Compare(animator.GetCurrAnimName(), "AttackToIdle2") == 0 ||
        //                    String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
        //                    String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
        //                animator.SetBool("JumpLoop", true);
        //            AudioSettings.PlaySFX("Jump");
        //        }
        //        else if (jumpCounter == 2)
        //        {
        //            if (String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
        //                        String.Compare(animator.GetCurrAnimName(), "Plunge") == 0)
        //            {
        //                animator.SetBool("AirDashStart", true);
        //            }
        //            AudioSettings.PlaySFX("DoubleJump");
        //        }

        //        rb.AddForce(jumpForce * Vector3.Up, ForceMode.Impulse);
        //        groundCheckLocked = true;
        //        grounded = false;

        //        Console.WriteLine("Jumps left " + (maxJumps - jumpCounter));
        //    }
        //}
        //private bool CanJump()
        //{
        //    return grounded || jumpCounter < maxJumps;
        //}
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
                    //if (!isAttacking)
                    //    &&
                    //    (String.Compare(animator.GetCurrAnimName(), "Walk") == 0 && input == Vector3.Zero) ||
                    //String.Compare(animator.GetCurrAnimName(), "Attack1") == 0 ||
                    //String.Compare(animator.GetCurrAnimName(), "Attack2") == 0 ||
                    //String.Compare(animator.GetCurrAnimName(), "Attack3ToLoco") == 0 ||
                    //    String.Compare(animator.GetCurrAnimName(), "JumpLoop") == 0 ||
                    //    String.Compare(animator.GetCurrAnimName(), "PlungeToIdle") == 0)
                    //animator.SetBool("Idle", true);
                }
            }
            //Console.WriteLine("Grounded: " + grounded);
        }
        //private IEnumerator Dash(Vector3 dashDir)
        //{
        //    isDashing = true;

        //    AudioSettings.PlaySFX("Dash");

        //    Vector3 dash = Vector3.Zero;

        //    animator.SetBool("AirDashStart", true);

        //    if (input == Vector3.Zero) dash = playerModel.GetComponent<Transform>().RotationQuat * transform.Backward * dashMultiplier;
        //    else dash = dashDir * dashMultiplier;

        //    rb.Velocity = new Vector3(dash.x, 0f, dash.z);

        //    yield return new WaitForSeconds(dashDuration);

        //    isDashing = false;
        //}
        //private void ExampleMovement()
        //{
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
        //}
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
            // transition to plunge if in air
            if (!grounded && isPlunging == false)
            {
                // do plunge?
                isPlunging = true;
                if (animator != null)
                {
                    animator.SetBool("Plunge", true);
                }
                return;
            }

            if (!isAttacking && !isPlunging)
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
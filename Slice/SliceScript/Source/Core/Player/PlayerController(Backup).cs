using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;


namespace SliceEngine
{

    public class PlayerControllerBackup : Entity, IInitializable
    {
        //public float rotationSpeed = 50.0f;
        //public string[] test3 = { "Test", "Test2" };
        //public Vector3[] TestVectors = { new Vector3(1, 1, 1),  new Vector3(2, 2, 2) };
        //public Vector3 direction = new Vector3(0.0f, 0.0f, 1.0f);
        //public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);
        //static bool testingShit = false;

        // =========== Debug Mode =========
        public bool debugMode = false;

        // =========== Debug Mod ==========


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
        private Vector3 finalMove;

        //private Vector3 velocity;
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
        private bool isGroundDashing = false, groundDashReady = true;
        private bool isAirDashing = false, airDashReady = true;
        private float dashTimer = 0f;
        private float groundDashCooldownUntil = 0f;
        private float airDashCooldownUntil = 0f;
        private Vector3 dashDir = Vector3.Zero;
        private float allowedDashDistance;

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
        public bool isAttacking = false;
        public float attackRecoveryDuration = 0.5f;
        private bool attackQueued = false;
        private bool attackAutoRecover = false;
        public float attack1Delay, attack2Delay, attack3Delay;

        public string attack1HBName;
        public int attack1Damage;
        public Vector3 attack1Window;
        public float attack1Duration;
        private GeneralHitbox attack1HB;

        public string attack2HBName;
        public int attack2Damage;
        public Vector3 attack2Window;
        public float attack2Duration;
        private GeneralHitbox attack2HB;

        public string attack3HBName;
        public int attack3Damage;
        public Vector3 attack3Window;
        public float attack3Duration;
        private GeneralHitbox attack3HB;

        private int attackIndex = 0;
        private float attackTimer = 0f;
        private bool queuedNext = false;

        private Coroutine attackCoroutine = null;

        // Chain-window facing override
        private bool queuedFacingOverride = false;
        private Vector3 queuedFacing = Vector3.Zero;

        // Lunge runtime (Atk1 / Atk2)
        private bool isLunging = false;
        private float lungeTimer = 0f;
        public float lungeDuration = 0f;
        private float lungeDelay = 0f;
        private bool lungeStarted = false;
        private Vector3 lungeDir = Vector3.Zero;
        public float lungeSpeed = 0f;

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
        public float plungeDuration = 0.5f;

        // =============== Internal variables =============== 
        public bool canInput = false;
        public bool canMove = false;
        private RigidBody rb;
        private GroundCheck groundCheck;
        private Animator animator;
        private CameraController camera;
        private AudioSource audio;
        private bool canIncrement = true;

        public bool canTeleport = false;

        public void Initialize()
        {
            camera = Bootstrap.CameraController;
            if (camera == null)
            {
                //console.writeline("Camera Var in player is EMPTY");
            }

            Bootstrap.HUDManager.SetHealth(currentHealth / maxHealth);
        }
        public override void OnCreate()
        {
            // NOTE: This shouldn't be true on create
            // cause if we implement tutorial they shouldnt be able to move on start
            canInput = true;
            canMove = true;
            if (debugMode)
            {
                return;
            }

            //console.writeline("Test");
            playerModel = gameObject.FindGameObjectWithName("RootNode");
            animator = playerModel?.GetComponent<Animator>();
            //if (animator == null) Console.WriteLine("No animator found");
            //else Console.WriteLine("Animator found");
            audio = gameObject.GetComponent<AudioSource>();
            rb = GetComponent<RigidBody>();
            //if (rb == null) Console.WriteLine("No rb found");
            //else Console.WriteLine("RB found");
            groundCheck = gameObject.FindGameObjectWithName("Ground Check")?.As<GroundCheck>();
            //if (groundCheck == null) Console.WriteLine("No ground check found");

            InitializeAttackHitboxes();
        }

        public override void OnUpdate(float dt)
        {
            Vector3 rbVel = rb.Velocity;
            //Console.WriteLine($"Velocity is {rbVel.x}, {rbVel.y}, {rbVel.z}");

            if (debugMode || canTeleport)
            {
                return;
            }

            GroundCheck();

            //if (canInput)
            {
                HandleInput();
                HandleDashInput();
            }
            if (attackQueued && !isAttacking)
            {
                ExecuteAttack();
                attackQueued = false;
                //console.writeline("AttackQueued set to false");
            }
            UpdateDash();
            if (canMove && !isAttacking && !attackAutoRecover) HandleMovement();
            HandleJump();
            AttackResetTimer();
            //Console.WriteLine($"Velocity is {velocity.x}, {velocity.y}, {velocity.z}");
            //Console.WriteLine($"Input is {input.x}, {input.y}, {input.z}");
        }
        private void HandleInput()
        {
            input = Vector3.Zero;
            // Forward/backward movement
            if (canInput)
            {
                if (Input.IsKeyDown(Keys.KEY_W)) input += new Vector3(0f, 0f, 1f);
                else if (Input.IsKeyDown(Keys.KEY_S)) input += new Vector3(0f, 0f, -1f);

                // Sideways movement 
                if (Input.IsKeyDown(Keys.KEY_A)) input += new Vector3(1f, 0f, 0f);
                else if (Input.IsKeyDown(Keys.KEY_D)) input += new Vector3(-1f, 0f, 0f);
            }


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
            if (canInput)
            {
                if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT)) TryAttack();
            }

        }
        #region New Movement
        private void HandleMovement()
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
            bool walkingNow = !isAttacking && !isGroundDashing && !isAirDashing && grounded && rawPlanarSpeed > 0.1f;
            //SliceLog.Log("GroundDashing: " + isGroundDashing.ToString() + " AirDashing: " + isAirDashing.ToString() + " isAttacking: " + isAttacking.ToString() + " isPlunging: " + isPlunging.ToString());
            if (isGroundDashing || isAirDashing)
            {
                float dashSpeed = isGroundDashing ? dashDistance / Math.Max(0.0001f, dashStartDuration)
                                                  : airDashDistance / Math.Max(0.0001f, airDashDuration);
                float distanceThisFrame = dashSpeed * Time.deltaTime;
                float moveAmount = Math.Min(allowedDashDistance, distanceThisFrame);

                if (allowedDashDistance >= 0f)
                {
                    allowedDashDistance -= moveAmount;
                }
                else
                {
                    if (isGroundDashing) EndGroundDash();
                    else if (isAirDashing) EndAirDash();
                }

                Vector3 dashVel = dashDir * moveAmount;
                finalMove = isGroundDashing
                    ? new Vector3(dashVel.x, dashVel.y + rb.Velocity.y, dashVel.z)
                    : dashVel;

                transform.Position += finalMove;

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
                        animator.SetBool("DashStart", true);
                    }
                    else if (isAirDashing)
                    {
                        animator.SetBool("AirDashStart", true);
                    }
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

                    Vector3 vel = rb.Velocity;
                    vel.y -= 1.0f;
                    rb.Velocity = vel;

                    // TODO: Add support for this later
                    //rb.Velocity.y = -1.0f; // physics crashes when I try to do any number thats too big for some reason
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
                    finalMove = new Vector3(0f, rb.Velocity.y, 0f);
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
                finalMove = new Vector3(horizontal.x, rb.Velocity.y, horizontal.z);
                transform.Position += finalMove * Time.deltaTime;
            }

            //Console.WriteLine($"Final move is x: {finalMove.x}, y: {finalMove.y}, z: {finalMove.z}");
        }

        // -------------------- Jump ------------------------------------------------------------------------------------------
        void HandleJump()
        {
            if (grounded)
            {

                lastGroundedTime = Time.time;
                Console.WriteLine(lastGroundedTime);
            }
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
            {
                lastJumpPressedTime = Time.time;
                Console.WriteLine("PRESSED SPACEBAR0");
            }

            bool canCoyote = (Time.time - lastGroundedTime) <= coyoteTime;
            bool bufferedJump = (Time.time - lastJumpPressedTime) <= jumpBuffer;

            if (bufferedJump) Console.WriteLine("BufferedJump is true");
            if (canCoyote) Console.WriteLine("Coyote is true");

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
                }
            }

            // Apply jump impulse (delayed if any)
            if (!isGroundDashing && !isAirDashing && !isAttacking && jumpRequested && !jumpImpulseApplied && Time.time >= jumpApplyAtTime)
            {
                float jumpSpeed = (float)Math.Sqrt(jumpHeight * -2f * gravity);
                Console.WriteLine("JUMPING");
                // TODO: CHange this to rb.Velocity.y = jumpSpeed once its supported
                Vector3 vel = rb.Velocity;
                vel.y = jumpSpeed;
                rb.Velocity = vel;

                jumpImpulseApplied = true;
                //AudioManager.instance.PlaySFX("Jump");
            }

            // Gravity
            if (isAirDashing)
            {
                // velocity.y = 0f;
                Vector3 vel = rb.Velocity;
                vel.y = 0.0f;
                rb.Velocity = vel;

            }
            else if (isPlunging && !plungeImpulseStarted)
            {
                Vector3 vel = rb.Velocity;
                vel.y = 0.0f;
                rb.Velocity = vel;
                //velocity.y = 0f;
            }
            else if (grounded && rb.Velocity.y <= 0)
            {
                // Hard lock to ground
                Vector3 vel = rb.Velocity;
                vel.y = 0f;
                rb.Velocity = vel;

                //velocity.y = 0f;
            }
            else
            {
                // Airborne → apply gravity
                // velocity.y += gravity * Time.deltaTime;
                //Vector3 vel = rb.Velocity;
                //vel.y = jumpSpeed;
                //rb.Velocity = vel;

            }


            wasGrounded = grounded;
        }

        void DoDoubleJump()
        {
            doubleJumpAvailable = false;
            jumpRequested = false;
            jumpImpulseApplied = true;

            if (resetYOnDoubleJump && rb.Velocity.y < 0f)
            {
                Vector3 velTemp = rb.Velocity;
                velTemp.y = 0.0f;
                rb.Velocity = velTemp;
            }
            Console.WriteLine("Double Jumping");
            float jumpSpeed = (float)Math.Sqrt(doubleJumpHeight * -2f * gravity);
            //velocity.y = jumpSpeed;
            Vector3 vel = rb.Velocity;
            vel.y = jumpSpeed;
            rb.Velocity = vel;


            if (animator != null)
            {
                animator.SetBool("AirDashStart", true);
            }
        }
        public void TeleportPlayer(Vector3 pos)
        {
            EndAttackState();
            transform.Position = pos;
        }
        #endregion

        // -------------------- Dash ------------------------------------------------------------------------------------------
        #region Dash
        void HandleDashInput()
        {
            if (!canInput) return;

            if (!Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_RIGHT)) return;

            // Dashes cancel the attack
            if (isAttacking)
            {
                EndAttackState();
            }

            if (grounded)
            {
                if (groundDashReady)
                {
                    BeginGroundDash();
                    StartCoroutine(DashCooldown());
                }
            }
            else
            {
                if (airDashReady)
                {
                    BeginAirDash();
                    StartCoroutine(AirDashCooldown());
                }
            }
        }
        private IEnumerator DashCooldown()
        {
            groundDashReady = false;
            yield return new WaitForSeconds(dashCooldown);
            groundDashReady = true;
        }
        private IEnumerator AirDashCooldown()
        {
            airDashReady = false;
            yield return new WaitForSeconds(airDashCooldown);
            airDashReady = true;
        }
        Vector3 ComputeFlatDashDir(bool useMoveDir)
        {
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
            //dashDir = AddUpwardAngle(flat, groundDashUpAngleDeg);

            Ray ray = new Ray(transform.Position, transform.Down);
            if (Physics.Raycast(ray, out RayCastHit hitInfo))
            {
                dashDir = Vector3.ProjectOnPlane(flat, hitInfo.normal);
            }

            allowedDashDistance = dashDistance;

            //console.writeline($"Creating new ray with direction {dashDir.x}, {dashDir.y}, {dashDir.z}"); 
            if (Physics.Raycast(transform.Position + new Vector3(0f, 2f, 0f), dashDir * 1000f, out RayCastHit dashHitInfo, LayerMask.ToMask("Environment"), QueryTriggerInteraction.UseGlobal))
            {
                if (allowedDashDistance >= dashHitInfo.distance)
                {
                    allowedDashDistance = dashHitInfo.distance * 0.98f;
                }
                //console.writeline($"Hit point at {dashHitInfo.point.x},{dashHitInfo.point.y},{dashHitInfo.point.z}");
            }
            dashTimer = Math.Max(0.0001f, dashStartDuration);

            //Vector3 flatFacing = transform.Forward;
            //flatFacing.y = 0f;
            //flatFacing.Normalize();
            //Vector3 flatDash = transform.Forward;
            //flatDash.y = 0f;
            //flatDash.Normalize();
            //float dot = Vector3.Dot(flatDash, flatFacing);
            //bool isBackDash = dot < backDashDotThreshold;


            if (animator != null)
            {
                //SliceLog.Log("Dash???");
                animator.SetBool("DashStart", true);
            }
        }

        void BeginAirDash()
        {
            isAirDashing = true;
            isGroundDashing = false;

            Vector3 flat = ComputeFlatDashDir(dashUsesMoveDirection);
            dashDir = AddUpwardAngle(flat, airDashUpAngleDeg);

            dashTimer = Math.Max(0.0001f, airDashDuration);
            //velocity.y = 0f;
            Vector3 velTemp = rb.Velocity;
            velTemp.y = 0.0f;
            rb.Velocity = velTemp;

            //Vector3 flatFacing = transform.Forward; flatFacing.y = 0f; flatFacing.Normalize();
            //Vector3 flatDash = dashDir; flatDash.y = 0f; flatDash.Normalize();
            //float dot = Vector3.Dot(flatDash, flatFacing);
            //bool isBackDash = dot < backDashDotThreshold;

            allowedDashDistance = airDashDistance;

            //console.writeline($"Creating new ray with direction {dashDir.x}, {dashDir.y}, {dashDir.z}");
            if (Physics.Raycast(transform.Position + new Vector3(0f, 2f, 0f), dashDir * 1000f, out RayCastHit dashHitInfo, LayerMask.ToMask("Environment"), QueryTriggerInteraction.UseGlobal))
            {
                if (allowedDashDistance >= dashHitInfo.distance)
                {
                    allowedDashDistance = dashHitInfo.distance * 0.98f;
                }
                //Console.WriteLine($"Hit point at {dashHitInfo.point.x},{dashHitInfo.point.y},{dashHitInfo.point.z}");
            }

            if (animator != null)
            {
                animator.SetBool("AirDashStart", true);
            }
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
            queuedFacingOverride = false;

            // Clear movement bursts
            isLunging = false;
            lungeStarted = false;

            atk3ArcActive = false;
            atk3ImpulseFired = false;
            atk3HorizVel = Vector3.Zero;
        }
        #endregion

        private void GroundCheck()
        {
            //if (groundCheckLocked)
            //{
            //    groundCheckTimer += Time.deltaTime;
            //    if (groundCheckTimer >= groundCheckDelay)
            //    {
            //        groundCheckLocked = false;
            //        groundCheckTimer = 0f;
            //    }
            //}

            //// Can only be grounded if initial delay is over
            //if (!groundCheckLocked)
            //{
            //    grounded = p// Gets grounded status from GroundCheck component
            //    if (grounded)
            //    {
            //        jumpCounter = 0;
            //    }
            //}
        }

        #region Attacks
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
            if (!grounded && isPlunging == false)
            {
                StartCoroutine(Plunge(plungeDuration));
                return;
            }
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
                        StartCoroutine(AttackDelay(attack1Delay, () => attack1HB.TurnOn()));

                        animator.SetBool("Attack1", true);
                        AudioSettings.PlaySFX("A1");

                        StartCoroutine(Lunge());
                        break;
                    case 2:
                        StartCoroutine(AttackDelay(attack1Delay, () => attack2HB.TurnOn()));

                        if (String.Compare(animator.GetCurrAnimName(), "Attack1") == 0)
                        {
                            animator.SetBool("Attack2", true);
                            AudioSettings.PlaySFX("A2");
                        }

                        StartCoroutine(Lunge());
                        break;
                    case 3:
                        StartCoroutine(AttackDelay(attack1Delay, () => attack3HB.TurnOn()));

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
        private IEnumerator Lunge()
        {
            float timer = 0f;
            while (timer < lungeDuration)
            {
                transform.Position += transform.Forward * lungeSpeed * Time.deltaTime;
                timer += Time.deltaTime;
                yield return null;
            }
        }
        private void TurnOffHitboxes()
        {
            attack1HB.TurnOff();
            attack2HB.TurnOff();
            attack3HB.TurnOff();
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
            isPlunging = true;
            if (animator != null)
            {
                animator.SetBool("Plunge", true);
            }
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
        private void Attack1(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attack1Damage, this.gameObject);
                //console.writeline("Hit enemy");
            }
            //console.writeline("Attack 1 executed");
        }
        private void Attack2(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attack2Damage, this.gameObject);
                //console.writeline("Hit enemy");
            }
            //console.writeline("Attack 2 executed");
        }
        private void Attack3(GameObject target)
        {
            EnemyBase enemy = target.As<EnemyBase>();
            if (enemy != null)
            {
                enemy.TakeDamage(attack3Damage, this.gameObject);
                //console.writeline("Hit enemy");
            }
            //console.writeline("Attack 3 executed");
        }
        private IEnumerator AttackDelay(float delay, Action action)
        {
            yield return new WaitForSeconds(delay);
            action.Invoke();
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
        public void CanAttackFlag(bool flag)
        {
            canIncrement = flag;
        }

        #endregion


        // ----------- Teleport --------------------

        public void Teleport(Vector3 toTeleport)
        {
            canTeleport = true;

            transform.Position = toTeleport;

            canTeleport = false;
        }

        #region On Overrides
        public override void OnCollideEnter(uint other)
        {
            // SliceLog.Log("OADMOSMODASM");
            // gameObject.Destroy();
        }
        public void OnGrounded()
        {

        }
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

    }
}
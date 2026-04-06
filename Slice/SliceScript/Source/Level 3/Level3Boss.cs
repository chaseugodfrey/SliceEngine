using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Level3Boss : EnemyBase
    {
        #region States
        public class IntroState : BaseState
        {
            /// <summary>
            /// Idk if you want to do any intro animation cutscene
            /// can probably handle it here
            /// </summary>
            Level3Boss bossController;
            IEnumerator moveCoroutine;

            bool setNarrative = false;
            bool isNarrativeDone = false;

            float timer = 0.0f;
            public IntroState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Intro State.");
                moveCoroutine = bossController.MoveToPoint(bossController.transform.Position, bossController.startingPosition, 3.0f);
                bossController.StartCoroutine(moveCoroutine);

                bossController.baseY = bossController.startingPosition.y;
            }
            
            public override void OnUpdate(float dt)
            {
                if (bossController.isMovementDone)
                {
                    //bossController.Bob(timer);

                    if (!setNarrative)
                        SetNarrative();

                    if (!isNarrativeDone)
                    {
                        if (Input.IsKeyPressed(Keys.KEY_F))
                        {
                            Bootstrap.HUDManager.PlayDialogueForLevel(0, Bootstrap.HUDManager.currentScene, false, true);
                        }

                        if (Bootstrap.HUDManager.dialogueIndex == 1)
                        {
                            bossController.bossSM.ChangeState(bossController.rechargingState);
                            bossController.Lvl3CutSceneManagerObj.As<Lvl3CutsceneManager>().CutToCam(1, 0.5f);
                            isNarrativeDone = true;
                        }
                    }

                    // EZE's Insertion
                    // Bobbing Code stolen from hafiz
                    Vector3 pos = bossController.transform.Position;
                    pos.y = bossController.baseY + Utilities.Sin(bossController.bobTimer * bossController.bobFrequency) * bossController.bobAmplitude;

                    bossController.transform.Position = pos;
                }

                timer += dt;
                bossController.transform.LookAt(Bootstrap.Player.GetComponent<Transform>().Position, Vector3.Up);


            }

            public override void OnExit()
            {

            }

            void SetNarrative()
            {
                setNarrative = true;
                bossController.Lvl3CutSceneManagerObj.As<Lvl3CutsceneManager>().CutToCam(0, 1f);
                Bootstrap.HUDManager.PlayDialogueForLevel(0, Bootstrap.HUDManager.currentScene, false, true);
            }
        }

        public class IdleState : BaseState
        {
            Level3Boss bossController;

            float timerMax = 3.0f;
            float timer;

            BaseState nextState;

            public IdleState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }
            public override void OnEnter()
            {
                SliceLog.Console("Idle State.");

                // check shields first first
                if (bossController.canRecharge && bossController.isShieldDestroyed)
                    nextState = bossController.rechargingState;
                else
                {
                    if (bossController.stateQueue.Count > 0)
                        nextState = bossController.stateQueue.Dequeue() as BaseState;
                    else
                        nextState = bossController.projectileState;
                }

                if (!bossController.canRecharge)
                    timerMax = 1f;

                timer = timerMax;
            }
            public override void OnUpdate(float dt)
            {
                // maybe can do telegraphing for attacks here
                // right now just look at player

                bossController.transform.LookAt(Bootstrap.Player.GetComponent<Transform>().Position, Vector3.Up);

                timer -= dt;
                if (timer <= 0.0f)
                {
                    bossController.bossSM.ChangeState(nextState);
                    nextState = null;
                }


                if (!bossController.isMovementDone)
                    return;

                // EZE's Insertion
                // Bobbing Code stolen from hafiz
                Vector3 pos = bossController.transform.Position;
                pos.y = bossController.baseY + Utilities.Sin(bossController.bobTimer * bossController.bobFrequency) * bossController.bobAmplitude;

                bossController.transform.Position = pos;

            }

            public override void OnExit()
            {
                bossController.isMovementDone = false;
            }
        }

        public class SummonState : BaseState
        {
            /// <summary>
            /// Idk if you want to do any intro animation cutscene
            /// can probably handle it here
            /// </summary>
            Level3Boss bossController;

            float timerMax = 3.0f;
            float timer;

            public SummonState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Summon State.");

                // ONLY IF HAVE 4 SPAWNERS
                Vector3[] offsets = new Vector3[]
                {
                    new Vector3(1, 1, 0),
                    new Vector3(-1, 1, 0),
                    new Vector3(0, 1, 1),
                    new Vector3(0, 1, -1)
                };

                bool first = bossController.projectileSpawners.Count == 0;

                for (int i = 0; i < 4; i++)
                {
                    GameObject go;

                    if (first)
                        go = bossController.CreateGameObject("Prefabs/AimingMechFollow.prefab");

                    else
                        go = bossController.projectileSpawners[i].gameObject;
                    
                    AimingMechFollow spawner = go.As<AimingMechFollow>();
                    spawner.GetComponent<Transform>().Position = owner.GetComponent<Transform>().Position;
                    spawner.followTarget = true;
                    spawner.SetTarget(Bootstrap.Player.GetComponent<Transform>());
                    spawner.offset = offsets[i] * 20.0f;
                    bossController.projectileSpawners.Add(spawner);
                }

                timer = timerMax;

                bossController.stateQueue.Enqueue(bossController.slamState);

                // EZE's Insertion
                // Bobbing Code stolen from hafiz
                Vector3 pos = bossController.transform.Position;
                pos.y = bossController.baseY + Utilities.Sin(bossController.bobTimer * bossController.bobFrequency) * bossController.bobAmplitude;

                bossController.transform.Position = pos;
            }

            public override void OnUpdate(float dt)
            {
                timer -= dt;
                if (timer <= 0.0f)
                {
                    bossController.bossSM.ChangeState(bossController.idleState);
                }
            }

            public override void OnExit()
            {
                timer = timerMax;
            }
        }

        public class SlamState : BaseState
        {
            Level3Boss bossController;
            public bool onCooldown = false;
            public bool attacking = false;
            public bool reset = false;
            public Vector3 ogPosition;
            float timer = 0.0f;

            bool isFiring = false;
            public SlamState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Entering slam state");
                onCooldown = false;
                attacking = false;
                reset = false;
                timer = 0.0f;

                Vector3 targetPos = Bootstrap.Player.GetComponent<Transform>().Position;
                targetPos.y = bossController.baseY;//owner.GetComponent<Transform>().Position.y;
                bossController.StartCoroutine(bossController.MoveToPoint(bossController.transform.Position, targetPos, 0.8f));
                ogPosition = targetPos;

                bossController.stateQueue.Enqueue(bossController.projectileState);
            }

            public override void OnUpdate(float dt)
            {
                if (bossController.isMovementDone && !attacking)
                {
                    attacking = true;

                    SliceLog.Console("Slamming");
                    owner.GetComponent<RigidBody>().gravityFactor = 40.0f;
                    if (!bossController.canRecharge && !isFiring)
                    {
                        Vector3 dir = (Bootstrap.Player.GetComponent<Transform>().WorldPosition - bossController.transform.WorldPosition).Normalize();
                        bossController.StartCoroutine(bossController.FireOrbitalLaserRow(bossController.transform.Position, dir, 10.0f, 5, 0.3f));
                        isFiring = true;
                    }
                }

                if (onCooldown)
                {
                    timer += dt;
                    bossController.isGrounded = true;

                    if (timer >= 0.5f)
                    {
                        bossController.ToggleHitbox(false);
                    }

                    if (timer >= 5.0f)
                    {
                        bossController.isGrounded = false;
                        onCooldown = false;
                        attacking = false;
                        reset = true;
                        ResetPosition();
                    }
                }
            }

            public void ResetPosition()
            {
                owner.GetComponent<RigidBody>().gravityFactor = 0.0f;
                bossController.StopAllCoroutines();
                bossController.StartCoroutine(bossController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, ogPosition, 0.8f));
                bossController.bossSM.ChangeState(bossController.idleState);
            }

            public override void OnExit()
            {
                owner.GetComponent<RigidBody>().gravityFactor = 0.0f;
                isFiring = false;
            }
        }

        public class RechargingState : BaseState
        {
            Level3Boss bossController;
            Vector3 rotDir = new Vector3(0, 1, 0);
            float rotSpeed = 10.0f;
            double rotTimer = 0.0;

            GameObject fx;
            bool startCharging = false;
            bool hasGen = true;
            bool isIntro = true;
            bool isLocked = true;

            public RechargingState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Recharging State.");

                bossController.isInvulnerable = true;
                bossController.isMovementDone = false;

                if (!isIntro)
                {
                    hasGen = bossController.canRecharge = bossController.SetupRecharging();
                    bossController.ReturnFollowingProjectiles();
                    bossController.stateQueue.Clear();
                }

                bossController.StartCoroutine(bossController.MoveToPoint(bossController.startingPosition, bossController.rechargingPosition, 2.4f));
            }

            public override void OnUpdate(float dt)
            {
                if (bossController.isMovementDone)
                {
                    if (!startCharging)
                    {
                        fx = bossController.CreateGameObject("Prefabs/FX_BossCharging.prefab");
                        startCharging = true;
                    }

                    if (hasGen)
                    {
                        rotTimer += dt;

                        if (rotTimer >= 0.5f)
                        {
                            //float x = SliceRandom.RangeFloat(0, 360);
                            //float y = SliceRandom.RangeFloat(0, 360);
                            //float z = SliceRandom.RangeFloat(0, 360);

                            //rotDir = new Vector3(x, y, z);
                            rotTimer = 0.0f;
                        }

                        hasGen = bossController.RechargeShield(isIntro);
                        // some silly animation for now

                        //bossController.transform.Rotation = rotDir;
                    }

                    else
                    {
                        bossController.canRecharge = bossController.shieldGeneratorManager.As<ShieldGeneratorManager>().CheckIfGeneratorsLeft();
                        bossController.stateQueue.Enqueue(bossController.summonState);
                        bossController.bossSM.ChangeState(bossController.idleState);
                    }

                    if (isIntro)
                    {
                        if (Input.IsKeyPressed(Keys.KEY_F) && isLocked)
                        {
                            isLocked = Bootstrap.HUDManager.PlayDialogueForLevel(0, 5, false, true);

                            if (!isLocked)
                            {
                                bossController.Lvl3CutSceneManagerObj.As<Lvl3CutsceneManager>().StopCutscene();
                                bossController.isIntroCutscene = false;
                            }
                        }
                    }    
                }
            }

            public override void OnExit()
            {
                bossController.StartCoroutine(bossController.MoveToPoint(bossController.rechargingPosition, bossController.startingPosition, 2.4f));

                isIntro = false;
                startCharging = false;
                bossController.isInvulnerable = false;

                fx.Destroy();
                fx = null;
            }
        }

        public class ProjectileState : BaseState
        {
            Level3Boss bossController;
            Transform playerTr;

            Vector3 destination;

            readonly float timerMax = 7f;
            readonly float moveSpeed = 10.0f;
            readonly float shootSpeed = 100.0f;
            readonly float shootRate = 0.1f;
            readonly int bulletDamage = 10;
            readonly float offsetRange = 5.0f;

            float timer = 0.0f;
            float shootTimer = 0.0f;

            readonly Vector3[] offsets =
            {
                Vector3.Up + Vector3.Left, Vector3.Up, Vector3.Up + Vector3.Right, 
                Vector3.Left, Vector3.Zero, Vector3.Right,
                Vector3.Down + Vector3.Left, Vector3.Down, Vector3.Down + Vector3.Right
            };

            Random rand;

            public ProjectileState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Projectile State.");
                
                playerTr = Bootstrap.Player.GetComponent<Transform>();

                timer = timerMax;
                CalculateDestination();

                rand = null;
                rand = new Random();

                //bossController.StartCoroutine(bossController.FireOrbitalLaserRandomRadius(playerTr.Position, 10.0f, 5, 0.25f));
                bossController.stateQueue.Enqueue(bossController.orbitalState);
            }

            public override void OnUpdate(float dt)
            {
                timer -= dt;

                if (timer <= 0)
                {
                    bossController.bossSM.ChangeState(bossController.idleState);
                }

                CalculateDestination();
                MoveToDestination();

                shootTimer -= dt;
                
                if (shootTimer <= 0.0f)
                {
                    FireBullet();
                    shootTimer = shootRate;
                }


                // EZE's Insertion
                // Bobbing Code stolen from hafiz
                Vector3 pos = bossController.transform.Position;
                pos.y = bossController.baseY + Utilities.Sin(bossController.bobTimer * bossController.bobFrequency) * bossController.bobAmplitude;

                bossController.transform.Position = pos;
            }

            public override void OnFixedUpdate(float dt)
            {

            }

            public override void OnExit()
            {

            }

            void FireBullet()
            {
                bossController.GetComponent<AudioSource>().Play();
                var bossTr = bossController.GetComponent<Transform>();
                Vector3 dirToPlayer = (playerTr.Position - bossController.transform.Position).Normalize();
                Vector3 finalPos = bossTr.Position + dirToPlayer * 2.0f + offsets[rand.Next(offsets.Length)] * offsetRange;
                GameObject go = bossController.CreateBullet(finalPos, bossTr.WorldRotationQuat.ToEuler(), Vector3.One, bulletDamage, shootSpeed, false, 1000.0f);
                go.As<Projectile>().destroyOnPlayerImpact = true;
            }

            void CalculateDestination()
            {
                destination = new Vector3(playerTr.Position.x, bossController.transform.Position.y, playerTr.Position.z);
            }

            void MoveToDestination()
            {
                bossController.transform.Position = Vector3.MoveTowards(bossController.transform.Position, destination, moveSpeed * Time.deltaTime);
                bossController.transform.LookAt(playerTr.Position, Vector3.Up);

                var player2DPos = new Vector3(playerTr.Position.x, bossController.transform.Position.y, playerTr.Position.z);
                float distance = Utilities.Distance3D(bossController.transform.Position, player2DPos);
                if (distance <= 15.0f)
                    bossController.bossSM.ChangeState(bossController.slamState); 
            }
        }

        public class OrbitalState : BaseState
        {
            Level3Boss bossController;
            bool isFiring = false;

            readonly float radius = 30.0f;

            int count = 0;

            public OrbitalState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Orbital State.");
                Vector3 finalPos = bossController.landingPositionObj.GetComponent<Transform>().WorldPosition;
                bossController.StopAllCoroutines();
                bossController.StartCoroutine(bossController.MoveToPoint(bossController.transform.Position, finalPos, 1.2f));
                //bossController.StartCoroutine(bossController.RotateToDir(new Vector3(-180.0f, 0, 0), 0.5f));
                bossController.isFiringDone = false;
            }

            public override void OnUpdate(float dt)
            {
                if (bossController.isMovementDone)
                {
                    if (!isFiring)
                    {
                        bossController.transform.LookAt(bossController.startingPosition, Vector3.Up);
                        if (bossController.canRecharge)
                            bossController.StartCoroutine(bossController.FireOrbitalLaserRandomRadius(bossController.transform.WorldPosition, radius, 5, 1.0f));
                        else
                        {
                            if (count < 1)
                            {
                                bossController.StartCoroutine(bossController.FireOrbitalLaserRandomRadius(bossController.transform.WorldPosition, radius, 10, 0.5f));
                                count++;
                            }

                            else
                            {
                                bossController.StartCoroutine(bossController.FireBigOrbitalLaser(Bootstrap.Player.transform.Position, 8.0f));
                                count = 0;
                            }

                        }

                        isFiring = true;
                    }


                    //bossController.transform.Rotation = new Vector3(-180.0f, bossController.transform.Rotation.y, bossController.transform.Rotation.z);
                    bossController.transform.Rotate(Vector3.Forward * dt * 100.0f);

                    if (bossController.isFiringDone)
                    {
                        bossController.StartCoroutine(bossController.MoveToPoint(bossController.transform.Position, bossController.startingPosition, 0.8f));
                        bossController.bossSM.ChangeState(bossController.idleState);
                    }
                }
            }

            public override void OnExit()
            {
                isFiring = false;
                bossController.isFiringDone = false;
                bossController.isMovementDone = false;
            }
        }

        public class DeathState : BaseState
        {
            Level3Boss bossController;
            bool triggerExplostion = false;
            float rotTimer = 0.0f;
            Vector3 rotDir;
            float moveTimer = 0.0f;
            float maxTimer = 1;

            AudioSource audioSource;
            public DeathState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                foreach(var spawner in bossController.projectileSpawners)
                {
                    spawner.DestroyChildren();
                }

                bossController.StopAllCoroutines();
                var cutsceneManager = bossController.Lvl3CutSceneManagerObj.As<Lvl3CutsceneManager>();
                cutsceneManager.StartCoroutine(cutsceneManager.DeathFadeInOut(bossController.transform));
                bossController.GetComponent<RigidBody>().gravityFactor = 0.0f;

                bossController.StartCoroutine(bossController.MoveToPoint(bossController.transform.WorldPosition, bossController.startingPosition, cutsceneManager.z_transitionDurationToDeath));

                audioSource = bossController.FindGameObjectsWithTag("BGMPlayer")[0].GetComponent<AudioSource>();
            }

            public override void OnUpdate(float dt)
            {
                if (bossController.isMovementDone)
                {
                    if (!triggerExplostion)
                    {
                        moveTimer += dt;

                        audioSource.Volume -= dt * 0.5f;
                        if (audioSource.Volume < 0.0f)
                            audioSource.Volume = 0.0f;

                        // rising
                        bossController.transform.Position += Vector3.Up * 1.0f * dt;

                        if (moveTimer > 7f)
                        {
                            audioSource.Volume = 0.0f;
                            audioSource.Stop();

                            var explosion = bossController.CreateGameObject("Prefabs/FX_FinalExplosion.prefab");
                            explosion.GetComponent<Transform>().Position = bossController.transform.Position;
                            triggerExplostion = true;
                        }
                    }

                    // shaking
                    rotTimer += dt;

                    if (rotTimer >= maxTimer)
                    {
                        float x = SliceRandom.RangeFloat(0, 360);
                        float y = SliceRandom.RangeFloat(0, 360);
                        float z = SliceRandom.RangeFloat(0, 360);

                        rotDir = new Vector3(x, y, z);
                        rotTimer = 0.0f;
                        maxTimer *= 0.75f;
                    }

                    // some silly animation for now

                    bossController.transform.Rotation = rotDir;
                }
            }

            public override void OnFixedUpdate(float dt)
            {

            }

            public override void OnExit()
            {

            }
        }

        public class EndState : BaseState
        {
            Level3Boss bossController;
            public EndState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {

            }

            public override void OnUpdate(float dt)
            {
                
            }

            public override void OnExit()
            {
            }


        }

        #endregion

        // make it public so u can access it in the states as well

        // declare all states here 
        public IntroState introState;
        public IdleState idleState;
        public SummonState summonState;
        public SlamState slamState;
        public RechargingState rechargingState;
        public ProjectileState projectileState;
        public OrbitalState orbitalState;
        public DeathState deathState;

        // state machine for the boss
        public StateMachine bossSM;

        public Queue stateQueue;
        List<Projectile> projectiles;
        public List<AimingMechFollow> projectileSpawners;
        public GameObject startingPositionObj;
        public GameObject landingPositionObj;
        public GameObject rechargePositionObj;
        public GameObject generalHitbox;
        public GameObject enemyHUD;
        public GameObject shieldGeneratorManager;

        public GameObject Lvl3CutSceneManagerObj;

        public Vector3 startingPosition;
        Vector3 rechargingPosition;

        public float currentShield = 100.0f;
        public float maxShield = 100.0f;
        public float restoreRate = 10.0f;

        public bool canRecharge = true;
        bool isInvulnerable = false;
        public bool isMovementDone = false;
        bool isFiringDone = false;
        bool isGrounded = false;
        bool isShieldDestroyed = false;
        bool isDead = false;
        bool isIntroCutscene = true;

        public float areaRadius = 100.0f;

        float[] thresholds = new float[] { 0.8f, 0.6f, 0.4f, 0.2f };
        int thresholdIndex = 0;

        public List<Coroutine> coroutines = new List<Coroutine>();

        // EZE'S INSERT FOR BOSS 2 CONSISTENCY
        // Bobbing
        float bobTimer = 0f;
        float bobAmplitude = 3.0f;
        float bobFrequency = 1.5f;
        float baseY = 0f;
        // EZE'S INSERT FOR BOSS 2 CONSISTENCY



        public override void OnCreate()
        {
            // initializing states and statemachine
            introState = new IntroState(this.gameObject);
            idleState = new IdleState(this.gameObject);
            slamState = new SlamState(this.gameObject);
            summonState = new SummonState(this.gameObject);
            rechargingState = new RechargingState(this.gameObject);
            projectileState = new ProjectileState(this.gameObject);
            orbitalState = new OrbitalState(this.gameObject);
            deathState = new DeathState(this.gameObject);
            bossSM = new StateMachine();
            stateQueue = new Queue();

            // initializing hitbox
            generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
            generalHitbox.As<GeneralHitbox>().TurnOff();

            // initializing values
            startingPosition = startingPositionObj.GetComponent<Transform>().WorldPosition;
            rechargingPosition = rechargePositionObj.GetComponent<Transform>().Position;

            //currentShield = maxShield;
            currentHealth = maxHealth;
            enemyHUD.As<Lvl3EnemyHUD>().SetHealth(currentHealth / maxHealth);
            enemyHUD.As<Lvl3EnemyHUD>().SetShield(currentShield / maxShield);

            projectileSpawners = new List<AimingMechFollow>();
            projectiles = new List<Projectile>();

        }

        public override void OnUpdate(float dt)
        {
            startingPosition = startingPositionObj.GetComponent<Transform>().WorldPosition;
            rechargingPosition = rechargePositionObj.GetComponent<Transform>().Position;

            // you have to call on update if u want the onUpdate to run
            Cheats();
            bossSM.OnUpdate(dt);
        }

        public override void OnFixedUpdate(float dt)
        { 
            // if u want fixed update calls in state machine
            bossSM.OnFixedUpdate(dt);
            bobTimer += dt;
        }

        public virtual IEnumerator MoveToPoint(Vector3 startPos, Vector3 targetPos, float duration)
        {
            float elapsedTime = 0.0f;
            Transform transform = this.gameObject.GetComponent<Transform>();
            isMovementDone = false;

            while (elapsedTime < duration)
            {
                elapsedTime += Time.deltaTime;
                float t = elapsedTime / duration;
                transform.Position = Vector3.Lerp(startPos, targetPos, t);
                yield return null;
                transform.LookAt(targetPos, Vector3.Up);
            }

            isMovementDone = true;
            transform.Position = targetPos;
        }

        public override void OnCollideEnter(uint other)
        {
            if (bossSM.currentState is SlamState slam)
            {
                if (!slam.onCooldown && slam.attacking)
                {
                    CreateGameObject("Prefabs/FX_TheBallSlam.prefab").GetComponent<Transform>().Position = transform.Position;
                    AudioSettings.PlaySFX("Smash");
                    Bootstrap.CameraController.Shake(0.2f, 4.0f);
                    ToggleHitbox(true);
                    //slam.ToggleHitbox(true);
                    slam.onCooldown = true;
                    SliceLog.Console("Hit the ground");
                }
            }
        }

        public virtual void ToggleHitbox(bool flag)
        {
            if (generalHitbox != null)
            {
                if (flag) generalHitbox.As<GeneralHitbox>().TurnOn();
                else generalHitbox.As<GeneralHitbox>().TurnOff();
            }
        }

        public virtual void DamagePlayer(GameObject hit)
        {
            if (hit.Has<PlayerController>())
            {
                Bootstrap.Player.TakeDamage(damage);
            }
        }

        public override void TakeDamage(int damage, GameObject source = null)
        {
            if (isInvulnerable)
                return;

            SliceLog.Console($"Boss took {damage} damage.");

            if (currentShield > 0)
            {
                currentShield -= damage;

                if (currentShield <= 0)
                    isShieldDestroyed = true;

                currentShield = Math.Max(currentShield, 0);
                OnDamaged(source);
            }
            else
            {
                //source = source ?? gameObject;
                if (source == null)
                {
                    source = gameObject;
                }
                //Console.WriteLine("Enitity taking damage");
                //Debug.Log($"{name} taking {amount} damage");
                this.currentHealth -= damage;
                if (this.currentHealth > 0) { OnDamaged(source); }
                if (this.currentHealth <= 0)
                {
                    currentHealth = 0; // Ensure health doesn't go below zero
                    OnDamaged(source);
                    OnDeath();
                }
            }
        }

        protected override void OnDamaged(GameObject source)
        {
            float hpPercent = (float)currentHealth / (float)maxHealth;
            enemyHUD.As<Lvl3EnemyHUD>().SetShield((float)currentShield / (float)maxShield);
            enemyHUD.As<Lvl3EnemyHUD>().SetHealth(hpPercent);

            if (thresholdIndex < thresholds.Length)
            {
                if (hpPercent <= thresholds[thresholdIndex])
                {
                    PlayPanicSFX();
                    thresholdIndex++;
                }
            }
        }

        public bool SetupRecharging()
        {
            SliceLog.Console("Setting up recharging phase.");
            bool hasGen = shieldGeneratorManager.As<ShieldGeneratorManager>().StartGenerators(1);
            return hasGen;
        }

        public bool RechargeShield(bool isIntro)
        {
            var shieldManager = shieldGeneratorManager.As<ShieldGeneratorManager>();
            bool hasGen = true;

            if (!isIntro)
                hasGen = shieldManager.RegenerateShields();

            if (!hasGen)
                return false;

            //SliceLog.Console("Recharging...");
            isShieldDestroyed = false;
            currentShield += restoreRate * Time.deltaTime * (isIntro ? 3.0f : 1.0f);
            currentShield = Math.Min(currentShield, maxShield);
            enemyHUD.As<Lvl3EnemyHUD>().SetShield(currentShield / maxShield);

            if (currentShield >= maxShield)
            {
                currentShield = maxShield;
                if (!isIntroCutscene)
                {
                    shieldManager.StopAllGenerators();
                    stateQueue.Enqueue(summonState);
                    bossSM.ChangeState(idleState);
                }
            }

            return true;
        }

        void ReturnFollowingProjectiles()
        {
            SliceLog.Console("Returning following projectiles to boss.");
            foreach (var spawner in projectileSpawners)
            {
                spawner.SetTarget(transform, 5.0f);
                spawner.offset = Vector3.Zero;
            }
        }

        public override void OnDeath()
        {
            AudioSettings.PlaySFX("05_03_Ozone_Death");
            isDead = true;
            isInvulnerable = true;
            bossSM.ChangeState(deathState);
            SliceLog.Console("Boss defeated!");
        }

        void Cheats()
        {
            if (Input.IsKeyPressed(Keys.KEY_L))
            {
                TakeDamage(1000);
            }

            if (Input.IsKeyPressed(Keys.KEY_J))
            {
                StopAllCoroutines();
                Lvl3CutSceneManagerObj.As<Lvl3CutsceneManager>().StopAllCoroutines();
                var manager = shieldGeneratorManager.As<ShieldGeneratorManager>();
                manager.DestroyAllGenerators();
                manager.StopAllCoroutines();

                TakeDamage(1000);
                TakeDamage(1000);

                canRecharge = false;
            }
        }

        IEnumerator FireBigOrbitalLaser(Vector3 position, float idleDuration)
        {
            GameObject go = CreateOrbitalLaser(80.0f, 100.0f, 0.0f, 5.0f, 0.2f);
            Vector3 finalPos = position;
            finalPos.y = go.GetComponent<Transform>().Position.y;
            go.GetComponent<Transform>().Position = position;

            yield return new WaitForSeconds(idleDuration);

            isFiringDone = true;

        }

        IEnumerator FireOrbitalLaserRow(Vector3 startPos, Vector3 dir, float distance, int count, float interval)
        {
            isFiringDone = false;

            while (count > 0)
            {
                GameObject go = CreateOrbitalLaser(10.0f, 100.0f, 1.0f, 2.5f, 2.0f);
                Transform tr = go.GetComponent<Transform>();
                float height = tr.Position.y;
                tr.Position = new Vector3(startPos.x, height, startPos.z);
                startPos += dir.Normalize() * distance;
                count--;
                yield return new WaitForSeconds(interval);
            }

            isFiringDone = true;
        }

        IEnumerator FireOrbitalLaserRandomRadius(Vector3 startPos, float radius, int count, float interval)
        {
            Random rand = new Random();
            while (count > 0)
            {
                Vector3 randomSphere = Utilities.RandomInsideSphere(radius);
                Vector3 finalPos = new Vector3(startPos.x + randomSphere.x, startPos.y, startPos.z + randomSphere.y);
                GameObject go = CreateOrbitalLaser(10.0f, 100.0f, 1.5f, 2.5f, 2.0f);
                Transform tr = go.GetComponent<Transform>();
                //float height = tr.WorldPosition.y;
                //tr.Position = new Vector3(finalPos.x, height, finalPos.z);
                tr.Position = finalPos;
                count--;
                yield return new WaitForSeconds(interval);
            }

            isFiringDone = true;
        }

        public GameObject CreateOrbitalLaser(float diameter, float height, float tracktime, float lifetime, float trackspeed)
        {
            GameObject laser = gameObject.CreateGameObject("Prefabs/OrbitalLaser.prefab");
            var laserScript = laser.As<OrbitalLaser>();

            laserScript.SetupLaser(diameter, height, tracktime, lifetime, trackspeed);  

            return laser;
        }

        public GameObject CreateBullet(Vector3 startPos, Vector3 angle, Vector3 scale, int damage, float speed, bool destroyOnImpact, float distanceBeforeDestroy)
        {
            string prefabName = "Projectile";
            string prefabPath = "Prefabs/" + prefabName + ".prefab";
            GameObject newBullet = gameObject.CreateGameObject(prefabPath);
            Transform tempT = newBullet.GetComponent<Transform>();

            tempT.Position = startPos;
            tempT.Rotation = angle;
            tempT.Scale = scale;

            Projectile tempP = newBullet.As<Projectile>();
            tempP.SetUp();
            tempP.speed = speed;
            tempP.owner = gameObject;
            tempP.damage = damage;
            tempP.distanceBeforeDestroy = distanceBeforeDestroy;
            tempP.destroyOnImpact = destroyOnImpact;

            projectiles.Add(tempP);

            //if (projectiles.Count > limit)
            //{
            //    for (int i = 0; i < (allProjectiles.Count - limit); i++)
            //    {
            //        DestroyBullet(allProjectiles[0]);
            //    }
            //}
            return newBullet;
        }

        public void DestroyBullet(Projectile toDestroy)
        {
            //int index = allProjectiles.IndexOf(toDestroy);
            //if (index != -1)
            //{
            //    Projectile temp = allProjectiles[index];
            //    allProjectiles.RemoveAt(index);
            //    temp.gameObject.Destroy();
            //}
        }

        IEnumerator RotateToDir(Vector3 targetDir, float duration)
        {
            Transform transform = this.gameObject.GetComponent<Transform>();
            Quaternion startRot = transform.RotationQuat;
            Quaternion targetRot = Quaternion.LookRotation(targetDir, Vector3.Up);
            float elapsedTime = 0.0f;
            while (elapsedTime < duration)
            {
                elapsedTime += Time.deltaTime;
                float t = elapsedTime / duration;
                transform.RotationQuat = Quaternion.Slerp(startRot, targetRot, t);
                yield return null;
            }
            transform.RotationQuat = targetRot;
        }

        IEnumerator TriggerExplosition(Vector3 position1, Vector3 position2, float waitTime1, float waitTime2)
        {
            yield return new WaitForSeconds(waitTime1);
            string prefab1 = "";
            gameObject.CreateGameObject(prefab1).GetComponent<Transform>().Position = position1;
            yield return new WaitForSeconds(waitTime2);
            string prefab2 = "";
            gameObject.CreateGameObject(prefab2).GetComponent<Transform>().Position = position2;
        }

        void PlayPanicSFX()
        {
            switch (thresholdIndex)
            {
                case 0:
                    AudioSettings.PlaySFX("05_02_Ozone_Panick_1");
                    break;
                case 1:
                    AudioSettings.PlaySFX("05_02_Ozone_Panick_2");
                    break;
                case 2:
                    AudioSettings.PlaySFX("05_02_Ozone_Panick_3");
                    break;
                case 3:
                    AudioSettings.PlaySFX("05_02_Ozone_Panick_4");
                    break;
                default:
                    break;
            }
        }
        void Bob(float time)
        {
            Vector3 finalPos = startingPosition;
            finalPos.y += Utilities.Sin(time * bobFrequency) * bobAmplitude;
            transform.Position = finalPos;
        }

        public void StartBoss()
        {
            bossSM.ChangeState(introState);
        }
    }
}

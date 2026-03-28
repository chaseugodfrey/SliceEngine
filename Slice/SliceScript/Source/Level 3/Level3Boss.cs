using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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

            float timer = 5.0f;
            public IntroState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Intro State.");
                moveCoroutine = bossController.MoveToPoint(bossController.transform.Position, bossController.startingPosition, 3.0f);
                bossController.StartCoroutine(moveCoroutine);
                bossController.stateQueue.Enqueue(bossController.summonState);
            }

            public override void OnUpdate(float dt)
            {
                timer -= dt;
                bossController.transform.LookAt(Bootstrap.Player.GetComponent<Transform>().Position, Vector3.Up);
                if (timer <= 0)
                {
                    bossController.bossSM.ChangeState(bossController.slamState);
                }
            }

            public override void OnExit()
            {
                CoroutineManager.StopCoroutine(moveCoroutine, bossController);
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
                timer = timerMax;
                nextState = bossController.stateQueue.Dequeue() as BaseState;
            }
            public override void OnUpdate(float dt)
            {
                // maybe can do telegraphing for attacks here
                // right now just look at player

                bossController.transform.LookAt(Bootstrap.Player.GetComponent<Transform>().Position, Vector3.Up);

                timer -= dt;
                if (timer <= 0.0f)
                {
                    if (nextState != null)
                    {
                        bossController.bossSM.ChangeState(nextState);
                        nextState = null;
                    }
                    else
                    {
                        SliceLog.Console("No next state queued, defaulting to summon state.");
                        bossController.bossSM.ChangeState(bossController.slamState);
                    }
                }
            }

            public override void OnExit()
            {

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

                bool first = bossController.projectileSpawners == null;
                if (first)
                    bossController.projectileSpawners = new List<Level3ProjectileSpawner>();

                for (int i = 0; i < 4; i++)
                {
                    GameObject go;

                    if (first)
                        go = bossController.CreateGameObject("Prefabs/Level3Projectile.prefab");

                    else
                        go = bossController.projectileSpawners[i].gameObject;
                    
                    Level3ProjectileSpawner spawner = go.As<Level3ProjectileSpawner>();
                    spawner.GetComponent<Transform>().Position = owner.GetComponent<Transform>().Position;
                    spawner.active = true;
                    spawner.followTarget = true;
                    spawner.SetTarget(Bootstrap.Player.GetComponent<Transform>());
                    spawner.offset = offsets[i] * 10;
                    bossController.projectileSpawners.Add(spawner);
                }

                timer = timerMax;
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
                targetPos.y = owner.GetComponent<Transform>().Position.y;
                bossController.StartCoroutine(bossController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, targetPos, 0.8f));
                ogPosition = targetPos;
            }

            public override void OnUpdate(float dt)
            {
                if (bossController.movementDone && !attacking)
                {
                    attacking = true;

                    SliceLog.Console("Slamming");
                    owner.GetComponent<RigidBody>().gravityFactor = 40.0f;
                }

                if (onCooldown)
                {
                    timer += dt;
                    bossController.grounded = true;

                    if (timer >= 0.5f)
                    {
                        bossController.ToggleHitbox(false);
                    }

                    if (timer >= 5.0f)
                    {
                        bossController.grounded = false;
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
                if (bossController.currentShield <= 0)
                    bossController.bossSM.ChangeState(bossController.rechargingState);
                else
                {
                    bossController.StartCoroutine(bossController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, ogPosition, 1.2f));
                    bossController.bossSM.ChangeState(bossController.idleState);
                }
            }

            public override void OnExit()
            {
                owner.GetComponent<RigidBody>().gravityFactor = 0.0f;
            }
        }


        public class RechargingState : BaseState
        {
            Level3Boss bossController;
            Vector3 rotDir = new Vector3(0, 1, 0);
            float rotSpeed = 10.0f;
            double rotTimer = 0.0;

            public RechargingState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                SliceLog.Console("Recharging State.");
                bossController.invulnerable = true;
                bossController.StartCoroutine(bossController.MoveToPoint(bossController.transform.Position, bossController.startingPosition, 1.0f));
                bossController.ReturnFollowingProjectiles();
            }

            public override void OnUpdate(float dt)
            {
                if (bossController.movementDone)
                {
                    rotTimer += dt;
                    bossController.RestoreShield();
                    // some silly animation for now

                    rotDir.x = (float)Math.Sin(rotTimer) + 1;
                    rotDir.z = (float)Math.Cos(rotTimer) + 1;
                    bossController.transform.Rotate(rotDir * rotSpeed * dt);
                }
            }

            public override void OnExit()
            {
                bossController.invulnerable = false;
            }
        }

        public class ReferenceState : BaseState
        {
            Level3Boss bossController;
            public ReferenceState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {

            }

            public override void OnUpdate(float dt)
            {

            }

            public override void OnFixedUpdate(float dt)
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
        public ReferenceState referenceState;

        // state machine for the boss
        public StateMachine bossSM;

        public Queue stateQueue;
        public List<Level3ProjectileSpawner> projectileSpawners;
        public GameObject startingPositionObj;
        public GameObject generalHitbox;
        public GameObject enemyHUD;
        Vector3 startingPosition;


        public float currentShield = 100.0f;
        public float maxShield = 100.0f;
        public float restoreRate = 10.0f;

        bool invulnerable = false;
        bool movementDone = false;
        bool grounded = false;

        public float areaRadius = 100.0f;

        public override void OnCreate()
        {
            introState = new IntroState(this.gameObject);
            idleState = new IdleState(this.gameObject);
            slamState = new SlamState(this.gameObject);
            summonState = new SummonState(this.gameObject);
            rechargingState = new RechargingState(this.gameObject);
            referenceState = new ReferenceState(this.gameObject);
            bossSM = new StateMachine();

            generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
            generalHitbox.As<GeneralHitbox>().TurnOff();

            startingPosition = startingPositionObj.GetComponent<Transform>().Position;
            // you have to set the state using
            stateQueue = new Queue();

            currentShield = maxShield;
            currentHealth = maxHealth;
            enemyHUD.As<Lvl3EnemyHUD>().SetHealth(currentHealth / maxHealth);
            enemyHUD.As<Lvl3EnemyHUD>().SetShield(currentShield / maxShield);

            bossSM.ChangeState(introState);
        }

        public override void OnUpdate(float dt)
        {
            // you have to call on update if u want the onUpdate to run
            bossSM.OnUpdate(dt);
            Cheats();

        }

        public override void OnFixedUpdate(float dt)
        { 
            // if u want fixed update calls in state machine
            bossSM.OnFixedUpdate(dt);
        }

        public virtual IEnumerator MoveToPoint(Vector3 startPos, Vector3 targetPos, float duration)
        {
            float elapsedTime = 0.0f;
            Transform transform = this.gameObject.GetComponent<Transform>();
            movementDone = false;

            while (elapsedTime < duration)
            {
                elapsedTime += Time.deltaTime;
                float t = elapsedTime / duration;
                transform.Position = Vector3.Lerp(startPos, targetPos, t);
                yield return null;
            }

            movementDone = true;
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
            SliceLog.Console($"Boss took {damage} damage.");
            if (invulnerable)
                return;

            if (currentShield > 0)
            {
                currentShield -= damage;
                currentShield = Math.Max(currentShield, 0);
                OnDamaged(source);
            }
            else
            {
                base.TakeDamage(damage);
            }
        }

        protected override void OnDamaged(GameObject source)
        {
            enemyHUD.As<Lvl3EnemyHUD>().SetShield((float)currentShield / (float)maxShield);
            enemyHUD.As<Lvl3EnemyHUD>().SetHealth((float)currentHealth / (float)maxHealth);
        }

        public void RestoreShield()
        {
            currentShield += restoreRate * Time.deltaTime;
            currentShield = Math.Min(currentShield, maxShield);
            enemyHUD.As<Lvl3EnemyHUD>().SetShield(currentShield / maxShield);

            if (currentShield >= maxShield)
            {
                currentShield = maxShield;
                stateQueue.Enqueue(summonState);
                bossSM.ChangeState(idleState);
            }
        }

        void ReturnFollowingProjectiles()
        {
            foreach (var spawner in projectileSpawners)
            {
                spawner.SetTarget(transform, 5.0f);
                spawner.offset = Vector3.Zero;
                spawner.active = false;
            }
        }

        public override void OnDeath()
        {
            SliceLog.Console("Boss defeated!");
        }

        void Cheats()
        {
            if (Input.IsKeyPressed(Keys.KEY_L))
            {
                TakeDamage(1000);
                if (currentShield <= 0)
                    bossSM.ChangeState(rechargingState);
            }
        }
    }
}

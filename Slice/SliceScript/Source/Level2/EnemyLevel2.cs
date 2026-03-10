using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    #region States
    public class IntroState : BaseState
    {
        EnemyLevel2 enemyController;

        // add a delay bfore hte enemy moves down
        float timer = 0.0f;
        float timeToMove = 2.0f;
        bool moved = false;

        public IntroState(GameObject owner, EnemyLevel2 controller) : base(owner)
        {
            enemyController = controller;
        }

        public override void OnEnter()
        {
            // when it enters, it will float down to the starting position
        }

        public override void OnUpdate(float dt)
        {
            timer += dt;
            if (timer >= 2.0f && !moved)
            {

                if (enemyController.startingPosition == null)
                {
                    SliceLog.Error("Starting position is null");
                    moved = true;
                }
                else
                {
                    enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().Position, 3.0f));
                    moved = true;
                }
            }
        }

        // transitions when movement is done in onMovementFinished in EnemyLevel2 
    }

    public class IdleState : BaseState
    {
        EnemyLevel2 enemyController;
        public int moves = 0;
        public bool firstPhase = true;

        public IdleState(GameObject owner, EnemyLevel2 controller) : base(owner)
        {
            enemyController = controller;
        }
        public override void OnEnter()
        {

            Console.WriteLine("Idle state entered");
            if (enemyController.stateMachine.prevState is SlamState)
            {
                moves = 0;

                // move straight away
                enemyController.movementTimer = enemyController.movementCooldown;

                // force it to move once atleast
                //enemyController.movementDone = false;
            }
        }

        public void FirstPhase(float dt)
        {
            if (owner != null)
            {
                // update movement for idle
                if (enemyController.movementDone)
                {
                        //Console.WriteLine("Incrementing");
                        // prob decide here if attack or no attack
                        // im not sure how to attack yet for now

                        // ill try this, % chance
                        enemyController.movementTimer += dt;
                }

                if (enemyController.movementTimer >= enemyController.movementCooldown && enemyController.movementDone)
                {
                    float roll = SliceRandom.RangeFloat(0.0f, 1.0f);
                    // 40% chance to slam attack
                    if (roll < 0.9f) // 75% chance for now cause testing
                    {
                        enemyController.stateMachine.ChangeState(enemyController.slamState);
                    }
                    else if (roll < 0.8f && roll > 0.4f)
                    {
                        // 40% chance to shoot something idk yet this the 2nd attack probably projectile based attack
                        Console.WriteLine("pew pew pew");
                        enemyController.stateMachine.ChangeState(enemyController.projectileState);
                    }
                    else
                    {
                        // nth, itll just move down and move to a new waypoint
                        enemyController.movementTimer = 0.0f;

                        if (enemyController.idlePoints.Count == 0)
                        {
                            SliceLog.Error("No idle points");
                        }
                        else
                        {
                            enemyController.currPoint = enemyController.GetNextIdlePoint();

                            enemyController.movementDone = false;
                            // move to the random point
                            enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, enemyController.idlePoints[enemyController.currPoint].GetComponent<Transform>().Position, 3.0f));

                        }
                    }

                }
            }
        }

        public void SecondPhase(float dt)
        {
            SliceLog.Console("Second phase updating");
        }

        public override void OnUpdate(float dt)
        {
           if(firstPhase)
           {
               FirstPhase(dt);
           }
           else
           {
               SecondPhase(dt);
           }

        }
    }

    public class SlamState : BaseState
    {
        EnemyLevel2 enemyController;
        public bool onCooldown = false;
        public bool attacking = false;
        public bool reset = false;
        public Vector3 originalPosition;
        float timer = 0.0f;

        public SlamState(GameObject owner, EnemyLevel2 controller) : base(owner)
        {
            enemyController = controller;
        }

        public override void OnEnter()
        {
            Console.WriteLine("Entering slam state");
            // reset all variables
            onCooldown = false;
            attacking = false;
            reset = false;
            timer = 0.0f;

            // move to the player fast
            Vector3 targetPos = Bootstrap.Player.GetComponent<Transform>().Position;
            targetPos.y = owner.GetComponent<Transform>().Position.y;
            enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, targetPos, 0.8f));
            //ToggleHitbox(true);
        }

        public override void OnUpdate(float dt)
        {
            // only start slamming once its done moving
            if (enemyController.movementDone && !attacking)
            {
                attacking = true;
                // save the original position before slamming
                //originalPosition = owner.GetComponent<Transform>().Position;

                // check if can slam 
                RayCastHit hitInfo;
                // Check if can plunge by raycasting down to see distance to environment layer objects
                bool hit = Physics.Raycast(owner.GetComponent<Transform>().Position + new Vector3(0, 1, 0), new Vector3(0, -1, 0) * 1000f, out hitInfo, LayerMask.GetMask("Environment"), QueryTriggerInteraction.UseGlobal);
                
                if (hit)
                {
                    GameObject objHit = owner.FindGameObjectWithID(hitInfo.transform.gameObject.mID);
                    if (objHit == null)
                    {
                        // no floor detected
                        enemyController.stateMachine.ChangeState(enemyController.projectileState);
                        return;
                    }

                }

                Console.WriteLine("Slamming");
                owner.GetComponent<RigidBody>().gravityFactor = 2.0f;
            }

            // onCooldown means it already hit the floor
            if (onCooldown)
            {
                timer += dt;
                enemyController.canDamage = true;

                if (timer >= 0.5f)
                {
                    // turn off hitbox?
                    enemyController.ToggleHitbox(false);
                }

                if (timer >= 10.0f)
                {
                    enemyController.canDamage = false;
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
            enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, originalPosition, 1.2f));
        }

        public override void OnExit()
        {
            owner.GetComponent<RigidBody>().gravityFactor = 0.0f;
        }
    }

    public class ProjectileState : BaseState
    {
        EnemyLevel2 enemyController;
        public List<Projectile> allProjectiles = new List<Projectile>();
        public float stateDuration = 5.0f;

        private float count = 0f;
        private float timer = 0f;

        public string projectilePrefabName = "Projectile";
        public float projPerSecond = 4f;
        public float bulletSpeed = 40f;
        public Vector3 bulletScale = new Vector3(1);
        public int bulletDamage = 10;
        public float distanceBeforeDestroyBullet = 90f;
        public int limit = 100;

        public ProjectileState(GameObject owner, EnemyLevel2 controller) : base(owner)
        {
            enemyController = controller;
        }
        public override void OnEnter()
        {
            // spawn projectiles from spawn points that shoot towards the player
            Console.WriteLine("Entering projectile state");
            timer = 0f;
            count = 0f;
        }
        public override void OnUpdate(float dt)
        {
            owner.GetComponent<Transform>().LookAt(Bootstrap.Player.transform.Position, new Vector3(0, 1, 0));

            timer += dt;
            count += dt;
            //Console.WriteLine($"count : {count}");
            if (count >= 1f / projPerSecond)
            {
                count -= 1f / projPerSecond;

                Transform T = owner.GetComponent<Transform>();
                //Console.WriteLine("SHooting boolet");
                GameObject bullet = CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed, false,distanceBeforeDestroyBullet);
                
                bullet.As<Projectile>().destroyOnPlayerImpact = true;
            }

            if (timer >= stateDuration)
            {
                enemyController.stateMachine.ChangeState(enemyController.idleState);
            }
        }

        public GameObject CreateBullet(Vector3 startPos, Vector3 angle, Vector3 scale, float speed, bool destroyOnImpact, float distanceBeforeDestroy)
        {
            string prefabPath = "Prefabs/" + projectilePrefabName + ".prefab";
            //GameObject newBullet = CreateGameObject("Prefabs/Projectile.prefab");
            GameObject newBullet = owner.CreateGameObject(prefabPath);

            Transform tempT = newBullet.GetComponent<Transform>();

            tempT.Position = startPos;
            tempT.Rotation = angle;
            tempT.Scale = scale;

            Projectile tempP = newBullet.As<Projectile>();

            tempP.SetUp();
            tempP.speed = speed;
            tempP.owner = owner;
            tempP.damage = bulletDamage;
            tempP.distanceBeforeDestroy = distanceBeforeDestroy;
            tempP.destroyOnImpact = destroyOnImpact;

            allProjectiles.Add(tempP);

            if (allProjectiles.Count > limit)
            {
                for (int i = 0; i < (allProjectiles.Count - limit); i++)
                {
                    DestroyBullet(allProjectiles[0]);
                }
            }

            return newBullet;
        }

        public void DestroyBullet(Projectile toDestroy)
        {
            int index = allProjectiles.IndexOf(toDestroy);

            if (index != -1)
            {
                Projectile temp = allProjectiles[index];
                allProjectiles.RemoveAt(index);
                temp.gameObject.Destroy();
            }
        }

    }

    public class DeathState : BaseState
    {
        EnemyLevel2 enemyController;

        List<GameObject> orbitingEnemies;
        float orbitTimer = 0.0f;
        float orbitRadius = 10.0f;
        float rotationSpeed = 2.0f;
        float idleTime = 3.0f;
        float idleTimer = 0.0f;
        int numOfPoints;

        // this is only for the very first move to center the enemy back to starting point
        public bool moved = false;
        // this is to prevent multiple MoveToPoitn coroutine calls.
        public bool secondMoved = false;



        public DeathState(GameObject owner) : base(owner)
        {
            enemyController = owner.As<EnemyLevel2>();
        }

        public override void OnEnter()
        {

            orbitingEnemies = enemyController.LevelController.As<L2Controller>().GetActiveProjectileEnemies();
            numOfPoints = orbitingEnemies.Count;

            for (int i = 0; i < numOfPoints; ++i)
            {
                float angle = i * (2.0f * (float)Math.PI / numOfPoints);

                Vector3 targetLocalPos = new Vector3(
                    (float)Math.Cos(angle) * orbitRadius,
                    0.0f,
                    (float)Math.Sin(angle) * orbitRadius
                    );

                orbitingEnemies[i].As<Projectile_Spawner>().active = false;

                // move back to the starting position
                Vector3 worldTarget = enemyController.startingPosition.GetComponent<Transform>().Position + targetLocalPos;
                enemyController.StartCoroutine(MoveEnemy(orbitingEnemies[i], worldTarget, 3.0f));
            }

            // Move back to the starting point
            enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().Position, 3.0f));
        }

        public IEnumerator MoveEnemy(GameObject enemy, Vector3 targetPos, float duration)
        {
            float elapsedTime = 0.0f;
            Transform enemyTransform = enemy.GetComponent<Transform>();
            Vector3 startPos = enemyTransform.Position;

            while (elapsedTime < duration)
            {
                elapsedTime += Time.deltaTime;
                float t = elapsedTime / duration;
                enemyTransform.Position = Vector3.Lerp(startPos, targetPos, t);
                yield return null;
            }

            enemyTransform.Position = targetPos;
        }

        public override void OnFixedUpdate(float dt)
        {
            
        }

        public override void OnUpdate(float dt)
        {
            if (moved)
            {
                orbitTimer += dt * rotationSpeed;

                idleTimer += dt;

                Vector3 center = owner.GetComponent<Transform>().Position;
                for(int i = 0; i < numOfPoints; ++i)
                {
                    float angle = i * (2.0f * (float)Math.PI / numOfPoints) + orbitTimer;

                    float x = center.x + (float)Math.Cos(angle) * orbitRadius;
                    float z = center.z + (float)Math.Sin(angle) * orbitRadius;

                    Transform enemyTransform = orbitingEnemies[i].GetComponent<Transform>();
                    enemyTransform.Position = new Vector3(x, center.y, z);
                }

                // after letting it sit in the middle for awhile
                // make it zoom up
                // im just gonna throw a random +Y value to curent pos
                if (idleTimer > idleTime && !secondMoved)
                {
                    secondMoved = true;
                    // Move back to the starting point
                    enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().Position + new Vector3(0, 100f, 0), 5.0f));
                }

            }
            
        }

        public override void OnExit()
        {
            
        }

    }

    #endregion

    public class EnemyLevel2 : EnemyBase
    {
        public StateMachine stateMachine;

        public IdleState idleState;
        public IntroState introState;
        public SlamState slamState;
        public ProjectileState projectileState;
        public DeathState deathState;

        public GameObject startingPosition;
        public GameObject enemyHUD;

        // Where it will move to when idle
        public List<GameObject> idlePoints = new List<GameObject>();
        public GameObject LevelController;
        public int currPoint = 0;
        public float movementCooldown = 5.0f;
        public float movementTimer = 0.0f;
        public bool movementDone = false;
        public int damage = 20;
        public bool canDamage = false;

        public GameObject generalHitbox;

        protected uint collidedEntity = 0;

        public override void OnCreate()
        {
            // Initialize state machine and states
            stateMachine = new StateMachine();
            idleState = new IdleState(this.gameObject);
            introState = new IntroState(this.gameObject);
            slamState = new SlamState(this.gameObject);
            projectileState = new ProjectileState(this.gameObject);
            deathState = new DeathState(this.gameObject); 
            // start at intro state
            stateMachine.ChangeState(introState);
            // start at a random point first also
            currPoint = GetNextIdlePoint();

            if (generalHitbox != null)
            {
                generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
                generalHitbox.As<GeneralHitbox>().TurnOff();
            }

            if (LevelController == null)
            {
                SliceLog.Error("Level controller isn't assigned");
            }

            enemyHUD.As<EnemyHUD>().SetHealth(currentHealth / maxHealth);
        }

        public virtual void DamagePlayer(GameObject hit)
        {
            Console.WriteLine("Damaging the player");
            if (hit.Has<PlayerController>())
            {
                Console.WriteLine("Player hit");
                Bootstrap.Player.TakeDamage(damage);

            }
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            //if (!canDamage)
            //    return;

            Console.WriteLine($"Take Damage called for {amount}");

            base.TakeDamage(amount, source);


        }

        public override void OnDeath()
        {
            // so the health bar drops to 0

            enemyHUD.As<EnemyHUD>().SetHealth((float)currentHealth / (float)maxHealth);
            // transition to the death state where it flies up
            //Console.WriteLine("Dying");
            stateMachine.ChangeState(deathState);
        }

        protected override void OnDamaged(GameObject source)
        {
            Console.WriteLine($"OnDamage for enemyLevel2 called: {currentHealth} and {maxHealth}");
            //CreateGameObject("Prefabs/Bloodsplatter.prefab").GetComponent<Transform>().Position = transform.Position;
            enemyHUD.As<EnemyHUD>().SetHealth((float)currentHealth / (float)maxHealth);

        }

        public override void OnUpdate(float dt)
        {
            stateMachine.OnUpdate(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            stateMachine.OnFixedUpdate(dt);
        }

        public virtual int GetNextIdlePoint()
        {
            // if theres only 1 point, then itll unfortunately have to stay at 1 position
            if (idlePoints.Count == 1)
                return 0;

            // get a random point to teleport to
            int nextPoint = SliceRandom.RangeInt(0, idlePoints.Count);
            while (nextPoint == currPoint)
            {
                nextPoint = SliceRandom.RangeInt(0, idlePoints.Count);
            }

            return nextPoint;
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
                yield return null; // Wait for the next frame
            }

            movementDone = true;
            // Ensure it ends exactly at the target position
            transform.Position = targetPos; 
            OnMovementFinish();
        }

        public virtual void OnMovementFinish()
        {
            Console.WriteLine("Movement Finished");
            switch(stateMachine.currentState)
            {
                // wtf is this syntax copilot auto filled this for me and it worked
                case IdleState _:
                    //movementDone = true;
                    IdleState idle = stateMachine.currentState as IdleState;
                    idle.moves++;
                    break;
                case IntroState _:
                    Console.WriteLine("Changing to idle State");
                    stateMachine.ChangeState(idleState);
                    break;
                case SlamState _:
                    SlamState slam = stateMachine.currentState as SlamState;
                    slam.originalPosition = transform.Position;
                    if (slam.reset)
                    {
                        stateMachine.ChangeState(idleState);
                    }
                    //Console.WriteLine($"Original position: {slam.originalPosition.ToString()}");
                    break;
                case DeathState _:
                    DeathState death = stateMachine.currentState as DeathState;
                    // the initial move for going back to starting point
                    death.moved = true;

                    // if secondMoved is true means this is when its done moving all the way up
                    // can probably transition to end level, go to level 3 here or smth idk
                    if (death.secondMoved)
                    {

                    }

                    break;
            }
        }

        public virtual void ToggleHitbox(bool flag)
        {
            //Console.WriteLine("Toggle hitbox");
            if (flag)
            {
                if (generalHitbox != null)
                {
                    //Console.WriteLine("Turning on hit box");
                    generalHitbox.As<GeneralHitbox>().TurnOn();
                }
            }
            else
            {
                if (generalHitbox != null)
                {
                    //Console.WriteLine("Turning off hitbox");
                    generalHitbox.As<GeneralHitbox>().TurnOff();
                }

            }
        }


        public override void OnCollideEnter(uint other)
        {
            // prevent multiple triggering
            if (collidedEntity == 0)
            {
                collidedEntity = other;
                if (stateMachine.currentState is SlamState slam)
                {
                    if (!slam.onCooldown && slam.attacking)
                    {
                        CreateGameObject("Prefabs/GroundSlamParticleFX.prefab").GetComponent<Transform>().Position = transform.Position - new Vector3(0, 2.0f, 0);
                        ToggleHitbox(true);
                        //slam.ToggleHitbox(true);
                        slam.onCooldown = true;

                        // idea: maybe let it sit for awhile so the player can do damage??

                        // make it rise back up once it hits the floor
                        //slam.ResetPosition();
                    }
                }
            }

        }

        public override void OnCollideExit(uint other)
        {
            if (collidedEntity == other)
            {
                collidedEntity = 0;
                if (stateMachine.currentState is SlamState slam)
                {
                    //slam.ToggleHitbox(false);
                }
            }
        }
    }
}

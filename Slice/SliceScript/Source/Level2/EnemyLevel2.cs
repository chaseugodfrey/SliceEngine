using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyLevel2 : EnemyBase
    {
        #region States

        public class IntroState : BaseState
        {
            EnemyLevel2 enemyController;

            // add a delay bfore hte enemy moves down
            float timer = 0.0f;
            float timeToMove = 2.0f;
            bool moved = false;

            float orbitTimer = 0.0f;
            float orbitRadius = 10.0f;
            float rotationSpeed = 2.0f;
            int numOfPoints;


            public IntroState(GameObject owner, EnemyLevel2 controller) : base(owner)
            {
                enemyController = controller;
            }

            public override void OnEnter()
            {
                // when it enters, it will float down to the starting position
                numOfPoints = enemyController.projectileShooters.Count;

                // turn off all projectile shooters for now
                foreach (GameObject shooter in enemyController.projectileShooters)
                {
                    shooter.As<Projectile_Spawner>().active = false;
                    shooter.As<Projectile_Spawner>().preAimObject.As<AlphaWiggleAnimation>().active = false;
                }
                // start at the starting point
                owner.GetComponent<Transform>().Position = enemyController.startingPosition.GetComponent<Transform>().WorldPosition;
            }

            public override void OnUpdate(float dt)
            {

                owner.GetComponent<Transform>().Position = enemyController.startingPosition.GetComponent<Transform>().WorldPosition;

                //timer += dt;
                //if (timer >= 5.0f && !moved)
                //{
                //    moved = true;
                //    //enemyController.stateMachine.ChangeState(enemyController.idleState);

                //    //if (enemyController.startingPosition == null)
                //    //{
                //    //    SliceLog.Error("Starting position is null");
                //    //    moved = true;
                //    //}
                //    //else
                //    //{
                //    //    //enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().WorldPosition, 3.0f));
                //    //    moved = true;
                //    //}
                //}

                orbitTimer += dt * rotationSpeed;
                Vector3 center = owner.GetComponent<Transform>().WorldPosition;
                for (int i = 0; i < numOfPoints; ++i)
                {
                    float angle = i * (2.0f * (float)Math.PI / numOfPoints) + orbitTimer;
                    float x = center.x + (float)Math.Cos(angle) * orbitRadius;
                    float z = center.z + (float)Math.Sin(angle) * orbitRadius;
                    Transform enemyTransform = enemyController.projectileShooters[i].GetComponent<Transform>();
                    enemyTransform.Position = new Vector3(x, center.y, z);
                }
            }

            public override void OnExit()
            {
                Vector3 center = owner.GetComponent<Transform>().WorldPosition;
                for (int i = 0; i < numOfPoints; ++i)
                {
                    float angle = i * (2.0f * (float)Math.PI / numOfPoints) + orbitTimer;
                    float x = (float)Math.Cos(angle) * (orbitRadius * 6);
                    float z = (float)Math.Sin(angle) * (orbitRadius * 6);

                    Vector3 worldTarget = center + new Vector3(x, 0f, z);
                    enemyController.StartCoroutine(enemyController.MoveEnemy(enemyController.projectileShooters[i], worldTarget, 1.5f));
                }

                enemyController.movementDone = true;

                AudioSettings.PlaySFX("04_01_Ozone_YouLeaveMeNoChoice");
            }
            // transitions when movement is done in onMovementFinished in EnemyLevel2 
        }

       

        public class IdleState : BaseState
        {
            EnemyLevel2 enemyController;
            public int moves = 0;
            // ray cast doesnt work that well if the user runs out of the ring so ill do distance from starting point instead
            public float distanceFromStarting = 50.0f;
            float timer = 0.0f;
            float timeToMove = 2.0f;
            public bool frozen = true;
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


                foreach (GameObject shooter in enemyController.projectileShooters)
                {
                    shooter.As<Projectile_Spawner>().active = true;
                    shooter.As<Projectile_Spawner>().preAimObject.As<AlphaWiggleAnimation>().active = true;
                }

            }

            public override void OnUpdate(float dt)
            {
                if (owner != null)
                {
                    // update movement for idle
                    if (enemyController.movementDone && !frozen)
                    {
                        //Console.WriteLine("Incrementing");
                        // prob decide here if attack or no attack
                        // im not sure how to attack yet for now

                        // ill try this, % chance
                        enemyController.movementTimer += dt;
                    }

                    // very first instance
                    if (frozen)
                    {
                        timer += dt;
                        if(timer > timeToMove)
                        {
                            frozen = false;
                        }

                    }


                    if (enemyController.movementTimer >= enemyController.movementCooldown && enemyController.movementDone)
                    {
                        float roll = SliceRandom.RangeFloat(0.0f, 1.0f);
                        // 40% chance to slam attack
                        if (roll < 0.6f) 
                        {
                            if (enemyController.startingPosition.GetComponent<Transform>().WorldPosition.Distance(Bootstrap.Player.transform.WorldPosition) > distanceFromStarting)
                            {
                                // if its too far from the center
                                // then transition to shooting instead
                                enemyController.stateMachine.ChangeState(enemyController.projectileState);
                                return;
                            }

                            enemyController.stateMachine.ChangeState(enemyController.slamState);
                        }
                        else if (roll < 0.8f && roll > 0.6f)
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
                                enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, enemyController.idlePoints[enemyController.currPoint].GetComponent<Transform>().WorldPosition, 3.0f));

                            }
                        }

                    }
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
                Vector3 targetPos = Bootstrap.Player.GetComponent<Transform>().WorldPosition;
                targetPos.y = owner.GetComponent<Transform>().WorldPosition.y;
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

                    //// check if can slam 
                    //RayCastHit hitInfo;
                    //// Check if can plunge by raycasting down to see distance to environment layer objects
                    //bool hit = Physics.Raycast(owner.GetComponent<Transform>().Position + new Vector3(0, 1, 0), new Vector3(0, -1, 0) * 1000f, out hitInfo, LayerMask.GetMask("Environment"), QueryTriggerInteraction.UseGlobal);

                    //if (hit)
                    //{
                    //    GameObject objHit = owner.FindGameObjectWithID(hitInfo.transform.gameObject.mID);
                    //    if (objHit == null)
                    //    {
                    //        // no floor detected
                    //        enemyController.stateMachine.ChangeState(enemyController.projectileState);
                    //        return;
                    //    }

                    //}


                    Console.WriteLine("Slamming");
                    owner.GetComponent<RigidBody>().gravityFactor = 40.0f;
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

            public string projectilePrefabName = "BallProjectile";
            public string shootFXPrefabName = "FX_Firing1";
            public float projPerSecond = 4f;
            public float bulletSpeed = 60f;
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
                    GameObject bullet = CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed, false, distanceBeforeDestroyBullet);

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
                string fxPrefabPath = "Prefabs/" + shootFXPrefabName + ".prefab";

                //GameObject newBullet = CreateGameObject("Prefabs/Projectile.prefab");
                GameObject newBullet = owner.CreateGameObject(prefabPath);
                GameObject firingEffect = owner.CreateGameObject(fxPrefabPath);

                Transform tempT = newBullet.GetComponent<Transform>();
                Transform tempT2 = firingEffect.GetComponent<Transform>();

                tempT.Position = startPos;
                tempT.Rotation = angle;
                tempT.Scale = scale;

                tempT2.Position = startPos;           

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

            

            public DeathState(GameObject owner, EnemyLevel2 controller) : base(owner)
            {
                enemyController = controller;
            }

            public override void OnEnter()
            {

                numOfPoints = enemyController.projectileShooters.Count;

                for (int i = 0; i < numOfPoints; ++i)
                {
                    float angle = i * (2.0f * (float)Math.PI / numOfPoints);

                    Vector3 targetLocalPos = new Vector3(
                        (float)Math.Cos(angle) * orbitRadius,
                        0.0f,
                        (float)Math.Sin(angle) * orbitRadius
                        );

                    enemyController.projectileShooters[i].As<Projectile_Spawner>().active = false;

                    // move back to the starting position
                    Vector3 worldTarget = enemyController.startingPosition.GetComponent<Transform>().WorldPosition + targetLocalPos;
                    enemyController.StartCoroutine(enemyController.MoveEnemy(enemyController.projectileShooters[i], worldTarget, 3.0f));
                }

                // Move back to the starting point
                enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().WorldPosition, 3.0f));
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
                    for (int i = 0; i < numOfPoints; ++i)
                    {
                        float angle = i * (2.0f * (float)Math.PI / numOfPoints) + orbitTimer;
                        float x = center.x + (float)Math.Cos(angle) * orbitRadius;
                        float z = center.z + (float)Math.Sin(angle) * orbitRadius;
                        Transform enemyTransform = enemyController.projectileShooters[i].GetComponent<Transform>();
                        enemyTransform.Position = new Vector3(x, center.y, z);
                    }

                    if (idleTimer > idleTime && !secondMoved)
                    {
                        secondMoved = true;
                        enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().WorldPosition + new Vector3(0, 100f, 0), 5.0f));
                    }
                }
            }

            public override void OnExit()
            {

            }

        }

        #endregion
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
        public List<GameObject> projectileShooters = new List<GameObject>();
        public int numOfProjectileShooters = 5;
        public string projectilePrefabPath = "Prefabs/ProjectileSpawnerLevel2.prefab";
        public GameObject generalHitbox;

        protected uint collidedEntity = 0;

        private int damageLeftTillSFX = 100;


        public override void OnCreate()
        {
            // Initialize state machine and states
            stateMachine = new StateMachine();
            idleState = new IdleState(this.gameObject, this);
            introState = new IntroState(this.gameObject, this);
            slamState = new SlamState(this.gameObject, this);
            projectileState = new ProjectileState(this.gameObject, this);
            deathState = new DeathState(this.gameObject, this);

            if (projectilePrefabPath.Length == 0)
            {
                SliceLog.Error("Projectile Prefab Path is empty!");

            }
            else
            {
                for (int i = 0; i < numOfProjectileShooters; ++i)
                {
                    GameObject newProjectileEnemy = CreateGameObject(projectilePrefabPath);
                    projectileShooters.Add(newProjectileEnemy);
                    newProjectileEnemy.GetComponent<Transform>().Position = gameObject.GetComponent<Transform>().Position;
                }
            }


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
                Bootstrap.Player.TakeDamage(damage, gameObject);

            }
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            //if (!canDamage)
            //    return;

            Console.WriteLine($"Take Damage called for {amount}");



            base.TakeDamage(amount, source);

            damageLeftTillSFX -= amount;
        }

        public override void OnDeath()
        {
            // so the health bar drops to 0

            enemyHUD.As<EnemyHUD>().SetHealth((float)currentHealth / (float)maxHealth);
            // transition to the death state where it flies up
            //Console.WriteLine("Dying");
            stateMachine.ChangeState(deathState);

            AudioSettings.PlaySFX("04_02_Ozone_InitiatingHailMAry");
        }

        protected override void OnDamaged(GameObject source)
        {
            Console.WriteLine($"OnDamage for enemyLevel2 called: {currentHealth} and {maxHealth}");
            CreateGameObject("Prefabs/FX_TheBallDamaged.prefab").GetComponent<Transform>().Position = transform.Position;
            enemyHUD.As<EnemyHUD>().SetHealth((float)currentHealth / (float)maxHealth); 

            if (damageLeftTillSFX <= 0 )
            {
                AudioSettings.PlaySFX("04_011_Ozone");

                damageLeftTillSFX += 100;
            }

        }

        public override void OnUpdate(float dt)
        {
            stateMachine.OnUpdate(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            stateMachine.OnFixedUpdate(dt);
        }

        public void TriggerState(string state)
        {
            switch(state)
            {
                case "Idle":
                    stateMachine.ChangeState(idleState);
                    break;
            }
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

        public IEnumerator MoveEnemy(GameObject enemy, Vector3 targetPos, float duration)
        {
            float elapsedTime = 0.0f;
            Transform enemyTransform = enemy.GetComponent<Transform>();
            Vector3 startPos = enemyTransform.WorldPosition;

            while (elapsedTime < duration)
            {
                elapsedTime += Time.deltaTime;
                float t = elapsedTime / duration;
                enemyTransform.Position = Vector3.Lerp(startPos, targetPos, t);
                yield return null;
            }

            enemyTransform.Position = targetPos;
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
                        Bootstrap.LevelDirector.LoadNextLevel();
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
                        CreateGameObject("Prefabs/FX_TheBallSlam.prefab").GetComponent<Transform>().Position = transform.Position;
                        AudioSettings.PlaySFX("Smash");
                        Bootstrap.CameraController.Shake(0.2f, 4.0f);
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

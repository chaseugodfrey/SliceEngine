using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyLevel3 : EnemyBase
    {
        #region States

        public class IntroState : BaseState
        {
            EnemyLevel3 enemyController;
            // add a delay bfore hte enemy moves down
            float timer = 0.0f;
            float timeToMove = 2.0f;
            bool moved = false;

            float orbitTimer = 0.0f;
            float orbitRadius = 10.0f;
            float rotationSpeed = 2.0f;
            int numOfPoints;

            public IntroState(GameObject owner, EnemyLevel3 controller) : base(owner)
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
                    //shooter.As<Projectile_Spawner>().preAimObject.As<AlphaWiggleAnimation>().active = false; // ALOY preAimObject is null when SpawnStyle is Nothing
                }
                // start at the starting point
                owner.GetComponent<Transform>().Position = enemyController.startingPosition.GetComponent<Transform>().WorldPosition;
                SliceLog.Console($"STarting pos: {enemyController.startingPosition.GetComponent<Transform>().WorldPosition.ToString()}");
            }

            public override void OnUpdate(float dt)
            {

                owner.GetComponent<Transform>().Position = enemyController.startingPosition.GetComponent<Transform>().WorldPosition;
                timer += dt;
                if (timer >= 5.0f && !moved)
                {
                    moved = true;
                    enemyController.stateMachine.ChangeState(enemyController.stasisState);

                    //if (enemyController.startingPosition == null)
                    //{
                    //    SliceLog.Error("Starting position is null");
                    //    moved = true;
                    //}
                    //else
                    //{
                    //    //enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().WorldPosition, 3.0f));
                    //    moved = true;
                    //}
                    SliceLog.Console("moved");
                }

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
                // on exitt, disperse them to surround the arena
                Vector3 center = owner.GetComponent<Transform>().WorldPosition;
                for (int i = 0; i < numOfPoints; ++i)
                {
                    float angle = i * (2.0f * (float)Math.PI / numOfPoints) + orbitTimer;
                    float x =(float)Math.Cos(angle) * (orbitRadius * 6);
                    float z = (float)Math.Sin(angle) * (orbitRadius * 6);

                    Vector3 worldTarget = center + new Vector3(x, 0f, z);
                    enemyController.StartCoroutine(enemyController.MoveEnemy(enemyController.projectileShooters[i], worldTarget, 1.5f));
                }
            }

        }

        public class StasisState : BaseState
        {
            EnemyLevel3 enemyController;
            public List<Projectile> allProjectiles = new List<Projectile>();
            private float count = 0f;
            private float timer = 0f;

            public string projectilePrefabName = "Projectile";
            public float projPerSecond = 4f;
            public float bulletSpeed = 40f;
            public Vector3 bulletScale = new Vector3(1);
            public int bulletDamage = 10;
            public float distanceBeforeDestroyBullet = 90f;
            public int limit = 100;
            public bool shieldFade = false;

            public StasisState(GameObject owner, EnemyLevel3 controller) : base(owner)
            {
                enemyController = controller;
            }
            public override void OnEnter()
            {
                SliceLog.Console("Entering stasis state");
                enemyController.shield = true;
                count = 0f;

                foreach (GameObject shooter in enemyController.projectileShooters)
                {
                    shooter.As<Projectile_Spawner>().active = true;
                    //shooter.As<Projectile_Spawner>().preAimObject.As<AlphaWiggleAnimation>().active = true; // ALOY preAimObject is null when SpawnStyle is Nothing

                }

            }

            public override void OnUpdate(float dt)
            {
                owner.GetComponent<Transform>().LookAt(Bootstrap.Player.transform.Position, new Vector3(0, 1, 0));
                //count += dt;

                //if (count >= 1f / projPerSecond)
                //{
                //    count -= 1f / projPerSecond;
                //    Transform T = owner.GetComponent<Transform>();
                //    GameObject bullet = CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed, false, distanceBeforeDestroyBullet);
                //    bullet.As<Projectile>().destroyOnPlayerImpact = true;
                //} Aloy here

                if (enemyController.shieldObject == null)
                {
                    SliceLog.Error("Shield Object not assigned");
                    return;
                }

                // once it can be damaged, meaning the shields are down
                // then transition to idle and continue the same behaviour as level 2
                if (enemyController.canDamage)
                {
                    if (enemyController.shield == false)
                    {
                        SliceLog.Console("Going to idle");
                        enemyController.stateMachine.ChangeState(enemyController.idleState);

                    }
                    else if (shieldFade == false)
                    {
                        SliceLog.Console("Starting coroutine to fade out shield");
                        enemyController.StartCoroutine(FadeOutShield(3.0f));
                        shieldFade = true;
                    }

                }
            }

            public IEnumerator FadeOutShield(float duration)
            {
                float elapsedTime = 0.0f;
                Vector4 col = enemyController.shieldObject.GetComponent<Renderer>().GetColor();
                Vector4 targetCol = col;
                targetCol.w = 0.0f;
                SliceLog.Console("Start of fade out shield");
                while (elapsedTime < duration)
                {
                    elapsedTime += Time.deltaTime;
                    float t = elapsedTime / duration;

                    //                    enemyTransform.Position = Vector3.Lerp(startPos, targetPos, t);
                    enemyController.shieldObject.GetComponent<Renderer>().SetColor(Vector4.Lerp(col, targetCol, t));
                    SliceLog.Console($"Current color of shield : {enemyController.shieldObject.GetComponent<Renderer>().GetColor()}");
                    yield return null;
                }

                SliceLog.Console("End of Fade out shield");

                enemyController.shieldObject.GetComponent<Renderer>().SetColor(targetCol);
                enemyController.shield = false;
                shieldFade = true;
            }

            public GameObject CreateBullet(Vector3 startPos, Vector3 angle, Vector3 scale, float speed, bool destroyOnImpact, float distanceBeforeDestroy)
            {
                string prefabPath = "Prefabs/" + projectilePrefabName + ".prefab";
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

            public override void OnExit()
            {
                enemyController.movementDone = true;
            }

        }

        public class IdleState : BaseState
        {
            EnemyLevel3 enemyController;
            public int moves = 0;
            // ray cast doesnt work that well if the user runs out of the ring so ill do distance from starting point instead
            public float distanceFromStarting = 50.0f;

            public IdleState(GameObject owner, EnemyLevel3 controller) : base(owner)
            {
                enemyController = controller;
            }

            public override void OnEnter()
            {
                SliceLog.Console("Idle state entered");
                if (enemyController.stateMachine.prevState is SlamState)
                {
                    moves = 0;
                    enemyController.movementTimer = enemyController.movementCooldown;
                }

                foreach (GameObject shooter in enemyController.projectileShooters)
                {
                    shooter.As<Projectile_Spawner>().active = true;
                    //shooter.As<Projectile_Spawner>().preAimObject.As<AlphaWiggleAnimation>().active = true; // ALOY preAimObject is null when SpawnStyle is Nothing

                }

                if (enemyController.stateMachine.prevState is StasisState)
                {
                    enemyController.movementDone = true;
                }
            }

            public override void OnUpdate(float dt)
            {
                if (owner != null)
                {
                    if (enemyController.movementDone)
                    {
                        enemyController.movementTimer += dt;
                    }

                    if (enemyController.movementTimer >= enemyController.movementCooldown && enemyController.movementDone)
                    {
                        float roll = SliceRandom.RangeFloat(0.0f, 1.0f);
                        if (roll < 0.4f)
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
                        else if (roll < 0.8f && roll > 0.4f)
                        {
                            SliceLog.Console("pew pew pew");
                            enemyController.stateMachine.ChangeState(enemyController.projectileState);
                        }
                        else
                        {
                            enemyController.movementTimer = 0.0f;
                            if (enemyController.idlePoints.Count == 0)
                            {
                                SliceLog.Error("No idle points");
                            }
                            else
                            {
                                enemyController.currPoint = enemyController.GetNextIdlePoint();
                                enemyController.movementDone = false;
                                enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, enemyController.idlePoints[enemyController.currPoint].GetComponent<Transform>().WorldPosition, 3.0f));
                            }
                        }
                    }
                }
            }
        }

        public class SlamState : BaseState
        {
            EnemyLevel3 enemyController;
            public bool onCooldown = false;
            public bool attacking = false;
            public bool reset = false;
            public Vector3 originalPosition;
            float timer = 0.0f;

            public SlamState(GameObject owner, EnemyLevel3 controller) : base(owner)
            {
                enemyController = controller;
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
                enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, targetPos, 0.8f));
            }

            public override void OnUpdate(float dt)
            {
                if (enemyController.movementDone && !attacking)
                {
                    attacking = true;
                    RayCastHit hitInfo;
                    bool hit = Physics.Raycast(owner.GetComponent<Transform>().Position + new Vector3(0, 1, 0), new Vector3(0, -1, 0) * 1000f, out hitInfo, LayerMask.ToMask("Environment"), QueryTriggerInteraction.UseGlobal);

                    if (hit)
                    {
                        GameObject objHit = owner.FindGameObjectWithID(hitInfo.transform.gameObject.mID);
                        if (objHit == null)
                        {
                            enemyController.stateMachine.ChangeState(enemyController.projectileState);
                            return;
                        }
                    }

                    SliceLog.Console("Slamming");
                    owner.GetComponent<RigidBody>().gravityFactor = 40.0f;
                }

                if (onCooldown)
                {
                    timer += dt;
                    enemyController.grounded = true;

                    if (timer >= 0.5f)
                    {
                        enemyController.ToggleHitbox(false);
                    }

                    if (timer >= 10.0f)
                    {
                        enemyController.grounded = false;
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
            EnemyLevel3 enemyController;
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

            public ProjectileState(GameObject owner, EnemyLevel3 controller) : base(owner)
            {
                enemyController = controller;
            }

            public override void OnEnter()
            {
                SliceLog.Console("Entering projectile state");
                timer = 0f;
                count = 0f;
            }

            public override void OnUpdate(float dt)
            {
                owner.GetComponent<Transform>().LookAt(Bootstrap.Player.transform.Position, new Vector3(0, 1, 0));
                timer += dt;
                count += dt;

                if (count >= 1f / projPerSecond)
                {
                    count -= 1f / projPerSecond;
                    Transform T = owner.GetComponent<Transform>();
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

        // death state will need to be changed
        public class DeathState : BaseState
        {
            EnemyLevel3 enemyController;
            float orbitTimer = 0.0f;
            float orbitRadius = 10.0f;
            float rotationSpeed = 2.0f;
            int numOfPoints;
            float idleTime = 3.0f;
            float idleTimer = 0.0f;
            public bool moved = false;
            public bool secondMoved = false;

            public DeathState(GameObject owner, EnemyLevel3 controller) : base(owner)
            {
                enemyController = controller;
            }

            public override void OnEnter()
            {
                // start cutscene
                Bootstrap.Player.SetPlayerLock(true);

                /*
                                //// Note: Ensure L2Controller access is still valid for Level 3 logic
                                //

                                //for (int i = 0; i < numOfPoints; ++i)
                                //{
                                //    float angle = i * (2.0f * (float)Math.PI / numOfPoints);

                                //    Vector3 targetLocalPos = new Vector3(
                                //        (float)Math.Cos(angle) * orbitRadius,
                                //        0.0f,
                                //        (float)Math.Sin(angle) * orbitRadius
                                //        );

                                //    enemyController.projectileShooters[i].As<Projectile_Spawner>().active = false;

                                //    // move back to the starting position
                                //    Vector3 worldTarget = enemyController.startingPosition.GetComponent<Transform>().WorldPosition + targetLocalPos;
                                //    enemyController.StartCoroutine(enemyController.MoveEnemy(enemyController.projectileShooters[i], worldTarget, 3.0f));
                                //}
                                // Move back to the starting point
                */
                
                Bootstrap.CameraController.SetFollowTarget(owner);

                
                enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, owner.GetComponent<Transform>().WorldPosition + new Vector3(0, 20.0f, 0), 4.0f));
                numOfPoints = enemyController.projectileShooters.Count;
                for (int i = 0; i < numOfPoints; i++)
                {
                    // destroy projectile shooters
                    enemyController.projectileShooters[i].Destroy();
                }

                enemyController.projectileShooters.Clear();
            }



            public override void OnUpdate(float dt)
            {
                if (moved)
                {
                    //orbitTimer += dt * rotationSpeed;
                    idleTimer += dt;
                    //Vector3 center = owner.GetComponent<Transform>().Position;
                    //for (int i = 0; i < numOfPoints; ++i)
                    //{
                    //    float angle = i * (2.0f * (float)Math.PI / numOfPoints) + orbitTimer;
                    //    float x = center.x + (float)Math.Cos(angle) * orbitRadius;
                    //    float z = center.z + (float)Math.Sin(angle) * orbitRadius;
                    //    Transform enemyTransform = enemyController.projectileShooters[i].GetComponent<Transform>();
                    //    enemyTransform.Position = new Vector3(x, center.y, z);
                    //}

                    if (idleTimer > idleTime && !secondMoved)
                    {
                        Bootstrap.LevelDirector.LoadNextLevel();
                        secondMoved = true;
                        //enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().WorldPosition + new Vector3(0, 100f, 0), 5.0f));
                    }
                }
            }
        }

        #endregion

        public StateMachine stateMachine;
        public IdleState idleState;
        public IntroState introState;
        public SlamState slamState;
        public ProjectileState projectileState;
        public DeathState deathState;
        public StasisState stasisState;

        public GameObject startingPosition;
        public GameObject enemyHUD;
        public List<GameObject> idlePoints = new List<GameObject>();
        //public GameObject LevelController;
        public int currPoint = 0;
        public float movementCooldown = 5.0f;
        public float movementTimer = 0.0f;
        public bool movementDone = false;
        public int damage = 20;
        public bool canDamage = false;
        public bool grounded = false;
        public GameObject generalHitbox;
        public GameObject shieldObject;
        public List<GameObject> projectileShooters = new List<GameObject>();
        public int numOfProjectileShooters = 5;
        public string projectilePrefabPath = "Prefabs/ProjectileSpawnerLevel2.prefab";
        protected uint collidedEntity = 0;

        public override void OnCreate()
        {
            stateMachine = new StateMachine();
            idleState = new IdleState(this.gameObject, this);
            introState = new IntroState(this.gameObject, this);
            slamState = new SlamState(this.gameObject, this);
            projectileState = new ProjectileState(this.gameObject, this);
            deathState = new DeathState(this.gameObject, this);
            stasisState = new StasisState(this.gameObject, this);

            if (projectilePrefabPath.Length == 0)
            {
                SliceLog.Error("Projectile Prefab Path is empty!");
                
            }
            else
            {
                for(int i = 0; i < numOfProjectileShooters; ++i)
                {
                    GameObject newProjectileEnemy = CreateGameObject(projectilePrefabPath);
                    projectileShooters.Add(newProjectileEnemy);
                    newProjectileEnemy.GetComponent<Transform>().Position = gameObject.GetComponent<Transform>().Position;
                }
            }

            stateMachine.ChangeState(introState);
            currPoint = GetNextIdlePoint();

            if (generalHitbox != null)
            {
                generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
                generalHitbox.As<GeneralHitbox>().TurnOff();
            }

            if (shieldObject == null)
            {
                SliceLog.Error("Shield object not assigned!");
            }

            //if (LevelController == null)
            //{
            //    SliceLog.Error("Level controller isn't assigned");
            //}

            enemyHUD.As<EnemyHUD>().SetHealth(currentHealth / maxHealth);
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

        public virtual void DamagePlayer(GameObject hit)
        {
            if (hit.Has<PlayerController>())
            {
                Bootstrap.Player.TakeDamage(damage);
            }
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            base.TakeDamage(amount, source);
        }

        public override void OnDeath()
        {
            enemyHUD.As<EnemyHUD>().SetHealth((float)currentHealth / (float)maxHealth);
            stateMachine.ChangeState(deathState);
        }

        protected override void OnDamaged(GameObject source)
        {
            enemyHUD.As<EnemyHUD>().SetHealth((float)currentHealth / (float)maxHealth);
        }

        public override void OnUpdate(float dt)
        {
            if (shieldObject == null)
            {
                SliceLog.Error("Shield object not assigned!");
            }

            stateMachine.OnUpdate(dt);
        }
        public override void OnFixedUpdate(float dt)
        {
            stateMachine.OnFixedUpdate(dt);
        }

        public virtual int GetNextIdlePoint()
        {
            if (idlePoints.Count == 1) return 0;
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
                yield return null;
            }

            movementDone = true;
            transform.Position = targetPos;
            OnMovementFinish();
        }

        public virtual void OnMovementFinish()
        {
            switch (stateMachine.currentState)
            {
                case IdleState idle:
                    idle.moves++;
                    break;
                case IntroState _:
                    //stateMachine.ChangeState(stasisState);
                    break;
                case SlamState slam:
                    slam.originalPosition = transform.Position;
                    if (slam.reset) stateMachine.ChangeState(idleState);
                    break;
                case DeathState death:
                    death.moved = true;
                    break;
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

        public override void OnCollideEnter(uint other)
        {
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
                    }
                }
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (collidedEntity == other)
            {
                collidedEntity = 0;
            }
        }
   
        public void ShieldGeneratorDestroyed()
        {
            canDamage = true;
        }
    }
}
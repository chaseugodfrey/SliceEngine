using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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

        public IntroState(GameObject owner) : base(owner)
        {
            enemyController = owner.As<EnemyLevel2>();
        }

        public override void OnEnter()
        {
            // when it enters, it will float down to the starting position
        }

        public override void OnUpdate(float dt)
        {
            timer += dt;
            if (timeToMove >= 2.0f && !moved)
            {
                enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().Position, enemyController.startingPosition.GetComponent<Transform>().Position, 3.0f));
                moved = true;
            }
        }

        // transitions when movement is done in onMovementFinished in EnemyLevel2 
    }

    public class IdleState : BaseState
    {
        EnemyLevel2 enemyController;
        public int moves = 0;


        public IdleState(GameObject owner) : base(owner)
        {
            enemyController = owner.As<EnemyLevel2>();
        }
        public override void OnEnter()
        {
            moves = 0;

            Console.WriteLine("Idle state entered");
            if (enemyController.stateMachine.prevState is SlamState)
            {
                // move straight away
                enemyController.movementTimer = enemyController.movementCooldown;

                // force it to move once atleast
                //enemyController.movementDone = false;
            }
        }

        public override void OnUpdate(float dt)
        {
            if (owner != null)
            {
                // update movement for idle
                if (enemyController.movementDone)
                {
                    // Console.WriteLine("Incrementing");
                    // prob decide here if attack or no attack
                    // im not sure how to attack yet for now

                    // ill try this, % chance
                        enemyController.movementTimer += dt;
                }

                if (enemyController.movementTimer >= enemyController.movementCooldown && enemyController.movementDone)
                {
                    float roll = SliceRandom.RangeFloat(0.0f, 1.0f);
                    // 40% chance to slam attack
                    if (roll < 0.75f && moves != 0) // 75% chance for now cause testing
                    {
                            enemyController.stateMachine.ChangeState(enemyController.slamState);
                    }
                    //else if (roll < 0.8f)
                    //{
                    //    // 40% chance to shoot something idk yet this the 2nd attack probably projectile based attack
                    //    Console.WriteLine("pew pew pew");

                    //}
                    else
                    {
                        // nth, itll just move down and move to a new waypoint
                        enemyController.movementTimer = 0.0f;
                        enemyController.currPoint = enemyController.GetNextIdlePoint();

                        enemyController.movementDone = false;
                        // move to the random point
                        enemyController.StartCoroutine(enemyController.MoveToPoint(owner.GetComponent<Transform>().transform.Position, enemyController.idlePoints[enemyController.currPoint].GetComponent<Transform>().Position, 3.0f));
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

        public SlamState(GameObject owner) : base(owner)
        {
            enemyController = owner.As<EnemyLevel2>();
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

                Console.WriteLine("Slamming");
                owner.GetComponent<RigidBody>().gravityFactor = 2.0f;
            }

            // onCooldown means it already hit the floor
            if (onCooldown)
            {
                timer += dt;

                if (timer >= 0.5f)
                {
                    // turn off hitbox?
                    enemyController.ToggleHitbox(false);
                }

                if (timer >= 2.0f)
                {
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

    }

    #endregion

    public class EnemyLevel2 : SliceBehaviour
    {
        public StateMachine stateMachine;

        public IdleState idleState;
        public IntroState introState;
        public SlamState slamState;

        public GameObject startingPosition;

        // Where it will move to when idle
        public List<GameObject> idlePoints = new List<GameObject>();
        public int currPoint = 0;
        public float movementCooldown = 5.0f;
        public float movementTimer = 0.0f;
        public bool movementDone = false;
        public int damage = 20;
        bool canDamage = false;

        public GameObject generalHitbox;

        uint collidedEntity = 0;

        public override void OnCreate()
        {
            // Initialize state machine and states
            stateMachine = new StateMachine();
            idleState = new IdleState(this.gameObject);
            introState = new IntroState(this.gameObject);
            slamState = new SlamState(this.gameObject);

            // start at intro state
            stateMachine.ChangeState(introState);
            // start at a random point first also
            currPoint = GetNextIdlePoint();

            if (generalHitbox != null)
            {
                generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
                generalHitbox.As<GeneralHitbox>().TurnOff();
            }
        }

        public void DamagePlayer(GameObject hit)
        {
            Console.WriteLine("Damaging the player");
            if (hit.Has<PlayerController>())
            {
                Console.WriteLine("Player hit");
                Bootstrap.Player.TakeDamage(damage);

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

        public int GetNextIdlePoint()
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

        public IEnumerator MoveToPoint(Vector3 startPos, Vector3 targetPos, float duration)
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

        public void OnMovementFinish()
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
            }
        }

        public void ToggleHitbox(bool flag)
        {
            Console.WriteLine("Toggle hitbox");
            if (flag)
            {
                if (generalHitbox != null)
                {
                    Console.WriteLine("Turning on hit box");
                    generalHitbox.As<GeneralHitbox>().TurnOn();
                }
            }
            else
            {
                if (generalHitbox != null)
                {
                    Console.WriteLine("Turning off hitbox");
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

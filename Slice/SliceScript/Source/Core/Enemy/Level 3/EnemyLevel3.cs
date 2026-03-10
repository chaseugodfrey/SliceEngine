using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    #region States

    #endregion

    public class EnemyLevel3 : EnemyLevel2
    {
        //this is scuffed as fk lol but the states need enemylevel2

        //public StateMachine stateMachine;

        //public IdleState idleState;
        //public IntroState introState;
        //public SlamState slamState;
        //public ProjectileState projectileState;

        //public GameObject startingPosition;

        //// Where it will move to when idle
        //public List<GameObject> idlePoints = new List<GameObject>();
        //public int currPoint = 0;
        //public float movementCooldown = 5.0f;
        //public float movementTimer = 0.0f;
        //public bool movementDone = false;
        ////public int damage = 20;
        //bool canDamage = false;

        //public GameObject generalHitbox;

        //uint collidedEntity = 0;

        public override void OnCreate()
        {
            // Initialize state machine and states
            stateMachine = new StateMachine();
            idleState = new IdleState(this.gameObject, this);
            introState = new IntroState(this.gameObject, this);
            slamState = new SlamState(this.gameObject, this);
            projectileState = new ProjectileState(this.gameObject, this);
            shield = true;

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

        public override void DamagePlayer(GameObject hit)
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

        public override int GetNextIdlePoint()
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

        public override IEnumerator MoveToPoint(Vector3 startPos, Vector3 targetPos, float duration)
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

        public  override void OnMovementFinish()
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

        public override void ToggleHitbox(bool flag)
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

        public override void TakeDamage(int amount, GameObject source = null)
        {
            //source = source ?? gameObject;
            if (source == null)
            {
                source = gameObject;
            }
            //Console.WriteLine("Enitity taking damage");
            //Debug.Log($"{name} taking {amount} damage");

            if (shield)
            {
                Console.WriteLine("LMAO have shield no damage for u");
            }
            else
            {
                this.currentHealth -= amount;
            }
                
            if (this.currentHealth > 0) { OnDamaged(source); }
            if (this.currentHealth <= 0)
            {
                currentHealth = 0; // Ensure health doesn't go below zero
                OnDeath();
            }
        }

        public void ShieldGeneratorDestroyed()
        {
            Console.WriteLine("Shield Generator Destroyeddddddd LMAO");
            shield = false;
        }
    }
}

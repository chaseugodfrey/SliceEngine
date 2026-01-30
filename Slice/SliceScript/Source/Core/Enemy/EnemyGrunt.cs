using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyGrunt : EnemyBase
    {
        public bool stunned = false;

        public float horKnockback = 1f;
        public float vertKnockback = 1f;

        public float strafeDistance = 10f;
        public float strafeTolerance = 2f;
        public float strafeSpeed = .4f;

        
        public float attackTriggerRange = 1f;
        public float attackDamageRange = 1f;
        public float flickerTiming = 1f;

        private bool isWinding = false; 
        public float attackWindUpTiming = 1f;
        public bool attacking { get; private set; } = false;
        private float _attackCounter = 0f;



        public GameObject basicHitBox;
        private GeneralHitbox _basicHitBox;



        //Function called when you want the enemy to be active
        public override void SetUp()
        {
            //Safety net return;
            if (active) return;


            base.SetUp(); 
            Console.WriteLine("Grunt setup called");

            this.ChangeState(new EnemyGruntChaseState(this));

            if(basicHitBox.Has<GeneralHitbox>())
            {
                _basicHitBox = basicHitBox.As<GeneralHitbox>();
                _basicHitBox.HitBoxListeners += BasicAttack;
                //_basicHitBox.SetActive(false);
                //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = false;

                _basicHitBox.TurnOff();

                #region Hitbox off Debug
                if (basicHitBox.GetComponent<ColliderShape>().ComponentEnabled == false)
                {
                    //Console.WriteLine("Hit Box successfully turned off");
                    SliceLog.Log("Hit Box successfully turned off");
                }
                else
                {
                    //Console.WriteLine("Hit Box still on");
                    SliceLog.Log("Hit Box still on");
                }
                #endregion
            }
            else
            {
                Console.WriteLine("Slime has no hitbox");
            }
        }

        public void Reset()
        {   active = false; }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyDown(Keys.KEY_O) && active == false)
            {
                Console.WriteLine("PPPPressed"); SetUp();
            }
            base.OnUpdate(dt);
        }


        #region Attacks
        public void BasicAttack(GameObject hit)
        {
            if( hit.Has<PlayerController>()  && hit.As<PlayerController>() == Bootstrap.Player)
            {
                //isPlayerInBasic = true;
                //RE INSERT ONCE ENABLE IS WORKING
                Bootstrap.Player.TakeDamage(damage);
            }
            else
            {
                Console.Write("| Failed player check on damage, no damage done |");
            }
        }

        public void StartWindUp()
        {
            StartCoroutine(WindUpCoroutine());
        }

        public void StopWindUp()
        {
            isWinding = false;
        }

        IEnumerator WindUpCoroutine()
        {
            float count = 0f;

            isWinding = true;

            while(count < attackWindUpTiming)
            {
                if (!isWinding)
                {
                    yield break;
                }

                count += Time.deltaTime;
                //Windup smt
                //Normally it should be an animation
            }

            ChangeState(new EnemyGruntAttackState(this));
            //attack

            yield break;
        }

        
        public void StartAttackCoroutine()
        {
            StartCoroutine(AttackCoroutine());
        }

        IEnumerator AttackCoroutine()
        {
            Console.Write("Attacking is On -> ");
            attacking = true;

            _basicHitBox.TurnOn();
            Console.Write("Box On | ");

            #region Collider Check Debug
            if (basicHitBox.GetComponent<ColliderShape>().ComponentEnabled == true)
            {
                Console.Write("Hit Box successfully turned on -> ");
                //SliceLog.Log("Hit Box successfully turned off");
            }
            else
            {
                Console.WriteLine("Hit Box still off -> ");
                //SliceLog.Log("Hit Box still on");
            }
            #endregion

            Console.Write("Flicker waiting -> ");
            yield return new WaitForSeconds(flickerTiming);
            Console.Write("Flicker returned -> ");


            _basicHitBox.TurnOff();
            Console.Write("Box Off | ");

            #region Collider Check debug
            if (basicHitBox.GetComponent<ColliderShape>().ComponentEnabled == false)
            {
                Console.WriteLine("Hit Box successfully turned off -> ");
                //SliceLog.Log("Hit Box successfully turned off");
            }
            else
            {
                Console.WriteLine("Hit Box still on -> ");
                //SliceLog.Log("Hit Box still on");
            }
            #endregion

            attacking = false;
            Console.WriteLine("Attacking is Off");

            yield break;
        }
        

        #endregion

        public override void TakeDamage(int amount, GameObject source = null)
        {
            // This override is just to insert a debug
            Console.WriteLine("Enemy is taking damage");
            SliceLog.Console("Enemy is taking damage");
            base.TakeDamage(amount, source);
        }


        #region On Override Methods
        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source)
        {
            rb.AddForce(new Vector3(0, vertKnockback, horKnockback), ForceMode.Impulse);
            ChangeState(new EnemyGruntStunnedState(this));
            SliceLog.Console("ENEMY IS BEING HIT");
        }


        //public override void OnDeath()
        //{

        //}
        #endregion
    }
}

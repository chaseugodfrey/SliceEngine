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

        public bool isWinding { get; private set; } = false;
        public float attackWindUpTiming = 1f;

        public bool triggerAttack { get; private set; } = false;
        public bool attacking { get; private set; } = false;
        private float _attackCounter = 0f;

        public float attackOdds = 0.1f;
        public float attackOddsCheckFrequency = 1f;

        public GameObject attackHitBoxRenderObject;
        public GameObject attackHitBoxObject;
        private GeneralHitbox attackHitBox;

        public GameObject windupSignalObject;
        public GameObject damagedSignal;

        public override void OnCreate()
        {
            base.OnCreate();
        }

        //Function called when you want the enemy to be active
        public override void SetUp()
        {
            //Safety net return;
            if (active) return;


            base.SetUp(); 
            Console.WriteLine("Grunt setup called");

            this.ChangeState(new EnemyGruntChaseState(this));

            if(attackHitBoxObject.Has<GeneralHitbox>())
            {
                attackHitBox = attackHitBoxObject.As<GeneralHitbox>();
                attackHitBox.HitBoxListeners += BasicAttack;
                //_basicHitBox.SetActive(false);
                //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = false;

                attackHitBox.TurnOff();
                attackHitBoxRenderObject.SetActive(false);
                windupSignalObject.SetActive(false);
                damagedSignal.SetActive(false);

                #region Hitbox off Debug
                if (attackHitBoxObject.GetComponent<ColliderShape>().ComponentEnabled == false)
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
            /*
            if (Input.IsKeyDown(Keys.KEY_O) && active == false)
            {
                Console.WriteLine("PPPPressed"); SetUp();
            }
            */
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
            isWinding = true;
            StartCoroutine(WindUpCoroutine());
            windupSignalObject.SetActive(true);
        }

        public void StopWindUp()
        {
            isWinding = false;
            windupSignalObject.SetActive(false);
        }

        IEnumerator WindUpCoroutine()
        {
            float count = 0f;


            while(isWinding && count < attackWindUpTiming)
            {

                count += Time.deltaTime;
                //Windup smt
                //Normally it should be an animation

                yield return new WaitForSeconds(Time.deltaTime);
            }


            //attack
            StopWindUp();

            triggerAttack = true;

            yield break;
        }

        
        public void StartAttackCoroutine()
        {
            triggerAttack = false;
            StartCoroutine(AttackCoroutine());
        }

        IEnumerator AttackCoroutine()
        {
            Console.Write("Attacking is On -> ");
            attacking = true;

            attackHitBox.TurnOn();
            attackHitBoxRenderObject.SetActive(true);
            Console.Write("Box On | ");

            #region Collider Check Debug
            if (attackHitBoxObject.GetComponent<ColliderShape>().ComponentEnabled == true)
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


            attackHitBox.TurnOff();
            attackHitBoxRenderObject.SetActive(false);
            Console.Write("Box Off | ");

            #region Collider Check debug
            if (attackHitBoxObject.GetComponent<ColliderShape>().ComponentEnabled == false)
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
            //SliceLog.Console("Enemy is taking damage");
            base.TakeDamage(amount, source);
        }


        #region On Override Methods
        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source)
        {
            if (rb == null)
            {
                Console.WriteLine("RigidBody is null, cannot apply knockback");
                return;
            }
            rb.AddForce(new Vector3(0, vertKnockback, horKnockback), ForceMode.Impulse);
           // ChangeState(new EnemyGruntStunnedState(this));
            SliceLog.Console("ENEMY IS BEING HIT");
        }


        //public override void OnDeath()
        //{

        //}
        #endregion
    }
}

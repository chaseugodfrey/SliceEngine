using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemySlime : EnemyBase
    {
        public bool stunned = false;

        public float horKnockback = 1f;
        public float vertKnockback = 1f;

        public float attackTriggerRange = 1f;
        public float attackDamageRange = 1f;
        public float attackWindUpTiming = 1f;
        public float flickerTiming = 1f;
        public bool exploding { get; private set; } = false;
        private float _explodingCounter = 0f;

        public GameObject basicHitBox;
        private GeneralHitbox _basicHitBox;



        //Function called when you want the enemy to be active
        public override void SetUp()
        {
            //Safety net return;
            if (active) return;


            base.SetUp(); 
            Console.WriteLine("Slime setup called");

            this.ChangeState(new EnemySlimeChaseState(this));

            if(basicHitBox.Has<GeneralHitbox>())
            {
                _basicHitBox = basicHitBox.As<GeneralHitbox>();
                _basicHitBox.HitBoxListeners += BasicAttack;
                //_basicHitBox.SetActive(false);
                //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = false;

                _basicHitBox.TurnOff();

                if(basicHitBox.GetComponent<ColliderShape>().ComponentEnabled == false)
                {
                    Console.WriteLine("Hit Box successfully turned off");
                    SliceLog.Log("Hit Box successfully turned off");
                }
                else
                {
                    Console.WriteLine("Hit Box still on");
                    SliceLog.Log("Hit Box still on");
                }
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
            if (Input.IsKeyDown(Keys.KEY_P) && active == false)
            {
                Console.WriteLine("PPPPressed"); SetUp();
            }
            base.OnUpdate(dt);
        }

        public void BasicAttack(GameObject hit)
        {
            Console.Write("| Basic Attack called |");

            if ( hit.Has<PlayerController>()  && hit.As<PlayerController>() == Bootstrap.Player)
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

        public void StartAttackCoroutine()
        {
            StartCoroutine(AttackCoroutine());
        }

        IEnumerator AttackCoroutine()
        {
            Console.Write("exploding is On -> ");
            exploding = true;

            Console.Write("Windup waiting -> ");
            yield return new WaitForSeconds(attackWindUpTiming);
            Console.Write("Windup returned -> ");

            // COMMENTING THIS OUT UNTIL ENABLE/DISABLE IS WORKING
            //_basicHitBox.SetActive(true);
            //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = true;
            _basicHitBox.TurnOn();
            Console.Write("Box On | ");


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


            Console.Write("Flicker waiting -> ");
            yield return new WaitForSeconds(flickerTiming);
            Console.Write("Flicker returned -> ");


            //_basicHitBox.As<GeneralHitbox>().SetActive(false);
            //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = false;
            _basicHitBox.TurnOff();
            Console.Write("Box Off | ");


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

            exploding = false;
            Console.WriteLine("exploding is Off");

            //ChangeState(new EnemySlimeChaseState(movementSpeed, attackTriggerRange));

            yield break;
        }


        public override void OnCollideEnter(uint other)
        {

        }

        public override void OnCollideStay(uint other)
        {

        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            // This override is just to insert a debug
            Console.WriteLine("Enemy is taking damage");
            SliceLog.Console("Enemy is taking damage");
            base.TakeDamage(amount, source);

        }



        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source)
        {
            rb.AddForce(new Vector3(0, vertKnockback, horKnockback), ForceMode.Impulse); 
            SliceLog.Console("ENEMY IS BEING HIT");
        }

        
        //public override void OnDeath()
        //{
            
        //}

    }
}

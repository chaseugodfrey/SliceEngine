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
        public bool attacking { get; private set; } = false;
        private float _attackCounter = 0f;

        public GameObject basicHitBox;
        private GeneralHitbox _basicHitBox;
        private bool isPlayerInBasic = false;

        //Function called when you want the enemy to be active
        public override void SetUp()
        {
            base.SetUp(); 
            Console.WriteLine("Slime setup called");
            this.ChangeState(new EnemySlimeChaseState(this));
            if(basicHitBox.Has<GeneralHitbox>())
            {
                _basicHitBox = basicHitBox.As<GeneralHitbox>();
                _basicHitBox.HitBoxListeners += BasicAttack;
                _basicHitBox.ExitListeners += TempRemovePlayerCheck;
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
            if (Input.IsKeyDown(Keys.KEY_P))
            {
                Console.WriteLine("PPPPressed"); SetUp();
            }
            base.OnUpdate(dt);
        }

        public void BasicAttack(GameObject hit)
        {
            if( hit.Has<PlayerController>()  && hit.As<PlayerController>() == Bootstrap.Player)
            {
                isPlayerInBasic = true;
                //RE INSERT ONCE ENABLE IS WORKING
                //Bootstrap.Player.TakeDamage(damage);
            }
        }

        public void TempRemovePlayerCheck(GameObject hit)
        {
            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                isPlayerInBasic = false;
                //Bootstrap.Player.TakeDamage(damage);
            }
        }

        public void StartAttackCoroutine()
        {
            StartCoroutine(AttackCoroutine());
        }

        IEnumerator AttackCoroutine()
        {
            Console.WriteLine("Attacking");
            attacking = true;

            yield return new WaitForSeconds(attackWindUpTiming);
            Console.WriteLine("Windup returned");
            // flicker on

            // COMMENTING THIS OUT UNTIL ENABLE/DISABLE IS WORKING
            //basicHitBox.As<GeneralHitbox>().SetActive(true);


            // COMMENTING THIS OUT UNTIL ENABLE/DISABLE IS WORKING
            //new WaitForSeconds(flickerTiming);
            //basicHitBox.As<GeneralHitbox>().SetActive(false);
            Console.WriteLine("Turning box On");

            //bool damaging = true;
            float count = 0f;
            while (count < flickerTiming)
            {
                count += Time.deltaTime;
                if (isPlayerInBasic)
                {
                    Bootstrap.Player.TakeDamage(damage);
                    break;
                }
            }


            Console.WriteLine("Turning box Off");
            // flicker off

            ///Outdated code that is just about checking distance.
            /// REMOVE ONCE SERIALIZED HIT BOX IS WORKING
            //Vector3 direction_diff = playerT.Position - enemyT.Position;
            //Console.WriteLine("Checking");
            //if (direction_diff.Magnitude() <= attackDamageRange)
            //{
            //    Console.WriteLine("Damage is through");
            //    Bootstrap.Player.TakeDamage(damage);
            //    //Make player take damage( waiting for rayan and jiale to do their thing)
            //}




            attacking = false;

            Console.WriteLine("attacking is Off");

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

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


//      public bool active = false;
        public bool stunned = false;

        public float horKnockback = 1f;
        public float vertKnockback = 1f;

        public float attackTriggerRange = 1f;
        public float attackCheckRange = 1f;
        public float attackWindUpTiming = 1f;
        //public float flickerTiming = 1f;
        public bool attacking { get; private set; } = false;
        private float _attackCounter = 0f;


        //private enum state 
        //{Chase,  Attack, Stunned};

        //private state currentState = state.Chase;





        //Function called when you want the enemy to be active

        public override void SetUp()
        { base.SetUp(); this.ChangeState(new EnemySlimeChaseState(this));}

        //public void Reset()
        //{   active = false; }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);
            //DoActionBasedOnState(dt);

            //if (Input.IsKeyDown(Keys.KEY_B))
            //{
            //    SetUp();
            //}

            //if (player.Has<Player>())
            //{
            //    PlayerController playerComp = player.As<PlayerController>();
            //    SliceLog.Log(playerComp.direction.ToString());
            //}

            //if (playerT.gameObject.Has<PlayerController>())
            //{
            //    PlayerController playerComp = playerT.gameObject.As<PlayerController>();
            //    SliceLog.Log(playerComp.direction.ToString());
            //}

        }

        private void DoActionBasedOnState(float deltaTime)
        {
            //switch (currentState) 
            //{
            //    case state.Chase:
            //        if (active && playerT != null && !stunned)
            //        {
            //            Vector3 direction_diff = playerT.Position - enemyT.Position;

            //            enemyT.Position += direction_diff.Normalize() * movementSpeed * deltaTime;

            //            if (direction_diff.Magnitude() <= attackTriggerRange)
            //            {
            //                currentState = state.Attack;
            //                //attack state
            //            }

            //        }
            //        break;
            //    case state.Attack:
            //        if (!attacking)
            //        {
            //            StartCoroutine(AttackCoroutine());
            //        }
            //        break;
            //    case state.Stunned:

            //        break;
            //} 
        }

        public void Attack()
        {
            StartCoroutine(AttackCoroutine());
        }

        IEnumerator AttackCoroutine()
        {
            attacking = true;

            yield return new WaitForSeconds(attackWindUpTiming);

            // flicker on

            Vector3 direction_diff = playerT.Position - enemyT.Position;

            if (direction_diff.Magnitude() <= attackCheckRange)
            {
                Bootstrap.Player.TakeDamage(damage);
                //Make player take damage( waiting for rayan and jiale to do their thing)
            }

            //new WaitForSeconds(flickerTiming);

            // flicker off

            attacking = false;

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

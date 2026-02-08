using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemySlimeOld : Entity
    {
        Transform enemyT;
        //GameObject player;
        Transform playerT = null;

        public bool active = false;
        public bool stunned = false;

        public float horKnockback = 1f;
        public float vertKnockback = 1f;

        public float attackRange = 1f;
        public float attackCheckRange = 1f;
        public float attackWindUpTiming = 1f;
        //public float flickerTiming = 1f;
        private bool attacking = false;
        //private float _attackCounter = 0f;

        private enum state 
        {Chase,  Attack, Stunned};

        private state currentState = state.Chase;

        private RigidBody rb;

        public override void OnCreate()
        {
            enemyT = GetComponent<Transform>();
            rb = GetComponent<RigidBody>();
        }

        //Function called when you want the enemy to be active
        public void SetUp()
        {   active = true;  playerT = Bootstrap.Player.transform; }

        public void Reset()
        {   active = false; }

        public override void OnUpdate(float dt)
        {
            DoActionBasedOnState(dt);

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
            switch (currentState) 
            {
                case state.Chase:
                    if (active && playerT != null && !stunned)
                    {
                        Vector3 direction_diff = playerT.Position - enemyT.Position;



                        enemyT.Position += direction_diff.Normalize() * movementSpeed * deltaTime;

                        if (direction_diff.Magnitude() <= attackRange)
                        {
                            currentState = state.Attack;
                            //attack state
                        }

                    }
                    break;
                case state.Attack:
                    if (!attacking)
                    {
                        StartCoroutine(AttackCoroutine());
                    }
                    break;
                case state.Stunned:

                    break;
            } 
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

            currentState = state.Chase;

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

        private bool isDead = false;

        public override void OnDeath()
        {
            if (!isDead)
            {
                isDead = true;
                this.gameObject.Destroy();
            }
        }

    }
}

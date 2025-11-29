using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemySlime : Entity
    {
        Transform enemyT;
        //GameObject player;
        Transform playerT = null;

        public bool active = false;
        public bool stunned = false;

        public float horKnockback = 1f;
        public float vertKnockback = 1f;

        public float attackRange = 1f;
        private bool attacking = false;

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
                            //attack state
                        }

                    }
                    break;
                case state.Attack:

                    break;
                case state.Stunned:

                    break;
            } 
        }


        public override void OnCollideEnter(uint other)
        {

        }

        public override void OnCollideStay(uint other)
        {

        }


        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source)
        { 
            rb.AddForce(new Vector3(0, vertKnockback, horKnockback), ForceMode.Impulse); 
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

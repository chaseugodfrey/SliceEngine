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

        public override void OnCreate()
        {
            enemyT = GetComponent<Transform>();
            //player = gameObject.FindGameObjectWithName("RootNode");
        }

        //Function called when you want the enemy to be active
        public void SetUp()
        {   active = true;  playerT = Bootstrap.Player.transform; }

        public void Reset()
        {   active = false; }

        public override void OnUpdate(float dt)
        {
            if (active && playerT != null)
            {
                Vector3 direction_diff = playerT.Position - enemyT.Position;

                enemyT.Position += direction_diff.Normalize() * movementSpeed * dt;
            }

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

        public override void OnCollideEnter(uint other)
        {
         //   SliceLog.Log("OADMOSMODASM");
            //gameObject.Destroy();
        }

        public override void OnCollideStay(uint other)
        {
          //  gameObject.Destroy();
        }


        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source)
        { 
            /*rb.AddForce(new Vector3(0, 1, 1), ForceMode.Impulse); */

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

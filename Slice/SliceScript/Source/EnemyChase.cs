using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyChase : SliceBehaviour
    {
        public float moveSpeed = 5.0f;
        public string targetObject;
        Transform targetTransform;
        Transform enemyT;
        //GameObject player;
        Transform playerT = null;
        public Vector3[] test;

        public override void OnCreate()
        {
            enemyT = GetComponent<Transform>();
            //player = gameObject.FindGameObjectWithName("RootNode");

            //if (player != null)
            //{
            playerT = gameObject.FindGameObjectWithName("RootNode").GetComponent<Transform>();
            //}

            //Hafiz: I'm not sure why this one is crashing the engine so I comment it out 29/12/2025
            //targetTransform = gameObject.FindGameObjectWithName(targetObject).GetComponent<Transform>();

        }

        public override void OnUpdate(float dt)
        {
            //if (playerT != null)
            //{
            //    Vector3 direction_diff = playerT.Position - enemyT.Position;

            //    enemyT.Position += direction_diff.Normalize() * moveSpeed * dt;
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

            if (targetTransform != null)
            {
                //Console.WriteLine("Has target Transform");
                if (HasComponent<NavAgent>())
                {
                   // Console.WriteLine("Has nav agent");
                    if (Input.IsKeyPressed(Keys.KEY_SPACEBAR))
                    {
                       // Console.WriteLine("Pressed backspace");
                        NavAgent agent = GetComponent<NavAgent>();
                        agent.SetDestination(targetTransform.Position);
                    }
                }
            }


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

    }
}

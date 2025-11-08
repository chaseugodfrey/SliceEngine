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

        Transform enemyT;
        GameObject player;
        Transform playerT;

        public override void OnCreate()
        {
            enemyT = GetComponent<Transform>();
            player = gameObject.FindGameObjectWithName("Player");
            playerT = player.GetComponent<Transform>();

            Console.WriteLine("ALOYSISU OIEVRE HERHhehehehehtest time x<" + playerT.Position.x + ">y<" + playerT.Position.y + ">z<" + playerT.Position.z);

        }

        public override void OnUpdate(float dt)
        {
            Vector3 direction_diff = playerT.Position - enemyT.Position;

            enemyT.Position += direction_diff.Normalize() * moveSpeed * dt;

        }

    }
}

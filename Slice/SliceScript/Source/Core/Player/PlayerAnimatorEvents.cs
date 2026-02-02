using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class PlayerAnimatorEvents : SliceBehaviour
    {
        public GameObject player;
        public override void OnCreate()
        {
            player = FindGameObjectsWithTag("Player")[0];
        }

        public void CanAttack(string str)
        {
            if(player.Has<PlayerController>())
            {
                player.As<PlayerController>().CanAttackFlag(true);
            }
        }
    }
}

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

        public void AttackRecovery(string str)
        {
            //Console.WriteLine("Attack recovery started.");
            Bootstrap.Player.StartAttackRecovery();
        }
        public void CanAttack(string str)
        {
            if(player.Has<PlayerController>())
            {
                player.As<PlayerController>().CanAttackFlag(true);
            }
        }
        public void SetAttacking(string state)
        {
            state = state.ToLower();

            switch (state)
            {
                case "true":
                    Bootstrap.Player.canMove = false;
                    Bootstrap.Player.isAttacking = true;
                    //Console.WriteLine("Player is now attacking.");
                    break;
                case "false":
                    Bootstrap.Player.canMove = true;
                    Bootstrap.Player.isAttacking = false;
                    //Console.WriteLine("Player is no longer attacking.");
                    break;
                default:
                    //Console.WriteLine("Invalid state for SetAttacking: " + state);
                    break;
            }
        }
    }
}

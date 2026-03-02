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
            if (player.Has<PlayerController>())
            {
                player.As<PlayerController>().StartAttackRecovery();
            }
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

            if (player.Has<PlayerController>())
            {
                PlayerController pc = player.As<PlayerController>();

                switch (state)
                {
                    case "true":
                        pc.playerCombatState = PlayerController.CombatState.Attacking;
                        break;
                    case "false":
                        pc.playerCombatState = PlayerController.CombatState.None;
                        break;
                }
            }
        }
        public void SetModelVisible(bool visible)
        {
            GameObject[] children = gameObject.GetAllChildren();
            float alpha = visible ? 1.0f : 0.0f;

            foreach (GameObject child in children)
            {
                if (child.HasComponent<Renderer>())
                {
                    Vector4 col = child.GetComponent<Renderer>().GetColor();
                    col.w = alpha;
                    child.GetComponent<Renderer>().SetColor(col);
                }
            }
        }
        public static void PlayPlayerSFX(string type)
        {
            type = type.ToLower();
            SliceLog.Log("wow");

            switch (type)
            {
                case "land":
                    AudioSettings.PlaySFX("Land");
                    SliceLog.Log("wow2");
                    break;
                case "plunge":
                    AudioSettings.PlaySFX("Plunge");
                    break;
                case "dash":
                    AudioSettings.PlaySFX("Dash");
                    break;
                case "jump":
                    AudioSettings.PlaySFX("Jump");
                    break;
                case "doublejump":
                    AudioSettings.PlaySFX("DoubleJump");
                    break;
            }
        }
    }
}

using SliceEngine;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;

namespace SliceEngine
{
    public class DamagePlayerOnContact : SliceBehaviour
    {
        public int damage = 1;
        public void DamagePlayer(GameObject hit)
        {

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                Bootstrap.Player.TakeDamage(damage, this.gameObject);
                AudioSettings.PlaySFX("PlayerHitLazer");
            }
        }

        public override void OnTriggerEnter(uint other)
        {
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null && collidedGO.tag == "Player")
            {
                DamagePlayer(collidedGO);
            }
        }
    }
}
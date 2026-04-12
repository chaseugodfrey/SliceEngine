using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;


namespace SliceEngine
{
    public class WallMechWall : SliceBehaviour
    {
        public int damage = 3;
        public void DamagePlayer(GameObject hit)
        {

            SliceLog.Log("Damage player called for projectiles");

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                SliceLog.Log("Player is hit");
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
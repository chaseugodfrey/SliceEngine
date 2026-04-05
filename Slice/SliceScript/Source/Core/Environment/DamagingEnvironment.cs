using SliceEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO.Pipes;
using System.Security.Permissions;


namespace SliceEngine
{
    public class DamagingEnvironment : SliceBehaviour
    {
        public int damage = 30;
        public GameObject generalHitboxObject;
        private GeneralHitbox generalHitbox;
        public bool destroyOnImpact = false;

        public void DamagePlayer(GameObject hit)
        {
            //SliceLog.Log("Damage player called for projectiles");

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                SliceLog.Log("Player is hit");
                Bootstrap.Player.TakeDamage(damage, this.gameObject);
            }
            if (destroyOnImpact)
            {
                this.gameObject.Destroy();
            }
        }
        public override void OnCreate()
        {
            base.OnCreate();

            //SliceLog.Log("Oncreatre start");
            if (generalHitboxObject.Has<GeneralHitbox>())
            {
                //SliceLog.Log("Hitbox scirpt exists");
                generalHitbox = generalHitboxObject.As<GeneralHitbox>();

               // SliceLog.Log("Grabbed script");
            }
            else
            {
                //SliceLog.Log("Hitbox scirpt does not exist");
            }
            //SliceLog.Log("Oncreatre end");
        }

        public override void OnAwake()
        {
            base.OnAwake();
            //SliceLog.Log("OnAwakw start");
            if (generalHitboxObject.Has<GeneralHitbox>())
            {
               //SliceLog.Log("Hitbox scirpt exists");
                generalHitbox = generalHitboxObject.As<GeneralHitbox>();

                //SliceLog.Log("Grabbed script");
            }
            else
            {
                //SliceLog.Log("Hitbox scirpt does not exist");
            }

            generalHitbox.HitBoxListeners += DamagePlayer;
            //SliceLog.Log("Added event");
            generalHitbox.TurnOn();
            //SliceLog.Log("turned on");
           // SliceLog.Log("OnAwake end");
        }
        /*
        public void SetUp()
        {
            generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
            generalHitbox.As<GeneralHitbox>().TurnOn();
        }
        */

        //public override void OnUpdate(float dt)
        //{
        //    base.OnUpdate(dt);

        //    //push it forward based on speed
        //}
    }
}
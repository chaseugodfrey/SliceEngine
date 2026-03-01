using SliceEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO.Pipes;
using System.Security.Permissions;


namespace SliceEngine
{
    public class Projectile : SliceBehaviour
    {
        public float speed = 1.0f;
        public int damage = 30;
        public GameObject generalHitbox;
        public Projectile_Spawner owner;

        public void DamagePlayer(GameObject hit)
        {

            SliceLog.Log("Damage player called for projectiles");

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                SliceLog.Log("Player is hit");
                Bootstrap.Player.TakeDamage(damage);
            }

            Destroy();
        }
        /*
        public override void OnCreate()
        {
            base.OnCreate();

            
        }
        */

        public void SetUp()
        {
            generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
            generalHitbox.As<GeneralHitbox>().TurnOn();
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (owner == null)
            {
                this.gameObject.Destroy();
            }

            //push it forward based on speed

            this.GetComponent<Transform>().Position += this.GetComponent<Transform>().Forward.Normalize() * speed * dt; 
        }

        public void Destroy()
        {
            if (owner != null)
            {
                owner.DestroyBullet(this);
            }
        }
    }
}
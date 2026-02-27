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
            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                Bootstrap.Player.TakeDamage(damage);
            }
        }
        public override void OnCreate()
        {
            base.OnCreate();

            //generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            //if (owner == null)
            //{
            //    this.gameObject.Destroy();
            //}

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
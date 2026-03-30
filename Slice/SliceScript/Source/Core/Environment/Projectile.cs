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
        //public GameObject generalHitbox;
        public GameObject owner;
        public bool destroyOnImpact = false;
        public bool destroyOnPlayerImpact = false;
        public float distanceBeforeDestroy = 10f;

        private float distancetravelledCount = 0f;
        public void DamagePlayer(GameObject hit)
        {

            SliceLog.Log("Damage player called for projectiles");

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                SliceLog.Log("Player is hit");
                Bootstrap.Player.TakeDamage(damage, this.gameObject);


                AudioSettings.PlaySFX("PlayerHitLazer");

               // CreateGameObject("Prefabs/FX_Hit.prefab").GetComponent<Transform>().Position = transform.Position;

                if (destroyOnPlayerImpact)
                {
                    DestroyProj();
                }
            }
            if (destroyOnImpact)
            {
                DestroyProj();
            }
        }
        public override void OnCreate()
        {
            base.OnCreate();
        }
        
        public void SetUp()
        {
            //generalHitbox.As<GeneralHitbox>().HitBoxListeners += DamagePlayer;
            //generalHitbox.As<GeneralHitbox>().TurnOn();
        }
        

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (owner == null)
            {
                this.gameObject.Destroy();
            }

            //push it forward based on speed

            float distanceTraveled = speed * dt;

            this.GetComponent<Transform>().Position += this.GetComponent<Transform>().Forward.Normalize() * distanceTraveled;

            distancetravelledCount += distanceTraveled;

            if (distancetravelledCount >= distanceBeforeDestroy)
            {
                DestroyProj();
            }
        }

        public void DestroyProj()
        {
            if (owner != null)
            {
                if (owner.Has<Projectile_Spawner>())
                    owner.As<Projectile_Spawner>().DestroyBullet(this);
                else if (owner.Has<EnemyLevel2>())
                    owner.As<EnemyLevel2>().projectileState.DestroyBullet(this);
                else if (owner.Has<SurroundAttack>())
                    owner.As<SurroundAttack>().DestroyBullet(this);
                else
                    gameObject.Destroy();
            }
        }

        public override void OnCollideEnter(uint other)
        {
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null)
            {
                if (collidedGO.tag == "Player")
                    DamagePlayer(collidedGO);
            }

            DestroyProj();
        }
    }
}
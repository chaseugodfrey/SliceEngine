using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class SurroundAttack : SliceBehaviour
    {
        // Config
        public int projectileCount = 5;
        public float ringRadius = 25f;
        public float telegraphDuration = 1.5f;
        public float bulletSpeed = 20f;
        public int bulletDamage = 10;
        public Vector3 bulletScale = new Vector3(1f);
        public float distanceBeforeDestroy = 75f;
        public string projectilePrefabName = "Projectile";
        public float ringYOffset = 1f;

        // How long to wait between full attack cycles
        public float attackCooldown = 8f;

        private List<Projectile> activeProjectiles = new List<Projectile>();
        private bool isAttacking = false;
        private float cooldownTimer = 0f;

        public override void OnCreate()
        {
            base.OnCreate();
            cooldownTimer = attackCooldown; // fire immediately on first cycle
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (isAttacking) return;

            cooldownTimer += dt;
            if (cooldownTimer >= attackCooldown)
            {
                cooldownTimer = 0f;
                StartCoroutine(SurroundAndFire());
            }
        }

        private IEnumerator SurroundAndFire()
        {
            isAttacking = true;

            // Snapshot the player position at the moment of attack
            Vector3 playerPos = Bootstrap.Player.transform.WorldPosition + new Vector3(0, ringYOffset, 0);

            // Spawn projectiles in a ring around the player, facing inward
            List<GameObject> spawnedBullets = new List<GameObject>();
            float angleStep = 360f / projectileCount;

            for (int i = 0; i < projectileCount; i++)
            {
                float angleRad = (i * angleStep) * ((float)Math.PI / 180f);
                Vector3 offset = new Vector3(
                    (float)Math.Cos(angleRad) * ringRadius,
                    0f,
                    (float)Math.Sin(angleRad) * ringRadius
                );

                Vector3 spawnPos = playerPos + offset;

                // Point inward toward player
                Vector3 inward = (playerPos - spawnPos).Normalize();
                float yaw = (float)Math.Atan2(inward.x, inward.z) * (180f / (float)Math.PI);
                Vector3 rotation = new Vector3(0f, yaw, 0f);

                GameObject bullet = SpawnBullet(spawnPos, rotation);
                // Disable movement until we fire — keep speed at 0 during telegraph
                bullet.As<Projectile>().speed = 0f;
                spawnedBullets.Add(bullet);
            }

            // Telegraph pause — projectiles sit still around the player
            yield return new WaitForSeconds(telegraphDuration);

            // Fire all inward at once
            foreach (GameObject bullet in spawnedBullets)
            {
                if (bullet == null) continue;
                Projectile p = bullet.As<Projectile>();
                if (p == null) continue;
                p.speed = bulletSpeed;
            }

            isAttacking = false;
        }

        private GameObject SpawnBullet(Vector3 position, Vector3 rotation)
        {
            string prefabPath = "Prefabs/" + projectilePrefabName + ".prefab";
            GameObject bullet = CreateGameObject(prefabPath);

            Transform t = bullet.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
            t.Scale = bulletScale;

            Projectile p = bullet.As<Projectile>();
            p.SetUp();
            p.speed = 0f;
            p.owner = this.gameObject;
            p.damage = bulletDamage;
            p.distanceBeforeDestroy = distanceBeforeDestroy;
            p.destroyOnImpact = false;
            p.destroyOnPlayerImpact = true;

            activeProjectiles.Add(p);
            return bullet;
        }

        public void DestroyBullet(Projectile toDestroy)
        {
            int index = activeProjectiles.IndexOf(toDestroy);
            if (index != -1)
            {
                activeProjectiles.RemoveAt(index);
                toDestroy.gameObject.Destroy();
            }
        }
    }
}
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
        public float ringRadius = 5f;
        public float telegraphDuration = 1.5f;
        public float bulletSpeed = 20f;
        public int bulletDamage = 10;
        public Vector3 bulletScale = new Vector3(1f);
        public float distanceBeforeDestroy = 30f;
        public string projectilePrefabName = "Projectile";
        public string preAimPrefabName = "FX_LaserPointer";
        public float ringYOffset = 1f;
        public float attackCooldown = 3f;

        // Pre-aim flicker settings (mirrors Projectile_Spawner)
        public float preAimFlickerRate = 10f;
        public float preAimMinAlpha = 0.3f;
        public float preAimMaxAlpha = 1f;
        public bool preAimRandom = false;

        private List<Projectile> activeProjectiles = new List<Projectile>();
        private List<GameObject> preAimObjects = new List<GameObject>();
        private bool isAttacking = false;
        private float cooldownTimer = 0f;

        public override void OnCreate()
        {
            base.OnCreate();
            //cooldownTimer = attackCooldown; // fire immediately on first cycle
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

            // Snapshot player position at moment of attack
            Vector3 playerPos = Bootstrap.Player.transform.WorldPosition + new Vector3(0, ringYOffset, 0);

            float angleStep = 360f / projectileCount;

            // Spawn pre-aim lines at each ring position pointing inward
            for (int i = 0; i < projectileCount; i++)
            {
                float angleRad = (i * angleStep) * ((float)Math.PI / 180f);
                Vector3 offset = new Vector3(
                    (float)Math.Cos(angleRad) * ringRadius,
                    0f,
                    (float)Math.Sin(angleRad) * ringRadius
                );

                Vector3 spawnPos = playerPos + offset;
                Vector3 rotation = GetInwardRotation(spawnPos, playerPos);

                // Spawn and orient the pre-aim line
                string preAimPath = "Prefabs/" + preAimPrefabName + ".prefab";
                GameObject preAim = CreateGameObject(preAimPath);
                Transform preAimT = preAim.GetComponent<Transform>();
                preAimT.Position = spawnPos;
                preAimT.Rotation = rotation;

                // Start flickering
                if (preAim.Has<AlphaWiggleAnimation>())
                {
                    AlphaWiggleAnimation a = preAim.As<AlphaWiggleAnimation>();
                    a.active = true;
                    a.rate = preAimFlickerRate;
                    a.MinWiggle = preAimMinAlpha;
                    a.MaxWiggle = preAimMaxAlpha;
                    a.random = preAimRandom;
                }

                preAimObjects.Add(preAim);
            }

            // Telegraph pause — lines sit and flicker
            yield return new WaitForSeconds(telegraphDuration);

            // Destroy all pre-aim lines
            foreach (GameObject preAim in preAimObjects)
            {
                if (preAim != null)
                    preAim.Destroy();
            }
            preAimObjects.Clear();

            //commented out for now in case i want to resnap to player but doesnt make sense lol
            //playerPos = Bootstrap.Player.transform.WorldPosition + new Vector3(0, ringYOffset, 0);



            // Spawn and immediately fire all projectiles inward
            for (int i = 0; i < projectileCount; i++)
            {
                float angleRad = (i * angleStep) * ((float)Math.PI / 180f);
                Vector3 offset = new Vector3(
                    (float)Math.Cos(angleRad) * ringRadius,
                    0f,
                    (float)Math.Sin(angleRad) * ringRadius
                );

                Vector3 spawnPos = playerPos + offset;
                Vector3 rotation = GetInwardRotation(spawnPos, playerPos);

                SpawnBullet(spawnPos, rotation);
            }

            isAttacking = false;
        }

        private Vector3 GetInwardRotation(Vector3 spawnPos, Vector3 targetPos)
        {
            Vector3 inward = (targetPos - spawnPos).Normalize();
            float yaw = (float)Math.Atan2(inward.x, inward.z) * (180f / (float)Math.PI);
            return new Vector3(0f, yaw, 0f);
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
            p.speed = bulletSpeed;
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
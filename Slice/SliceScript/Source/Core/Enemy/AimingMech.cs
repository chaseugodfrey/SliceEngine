using SliceEngine;
using System.Collections.Generic;

namespace SliceEngine
{
    public class AimingMech : SliceBehaviour
    {
        public List<Projectile> allProjectiles = new List<Projectile>();

        // Bullet settings
        public string projectilePrefabName = "Projectile";
        public float projPerSecond = 10.0f;
        public float bulletSpeed = 50.0f;
        public Vector3 bulletScale = new Vector3(1);
        public int bulletDamage = 1;
        public float distanceBeforeDestroyBullet = 250.0f;

        // Aim settings
        public float aimVerticalOffset = 1f;

        public bool active = false;

        private float count = 0f;

        // Burst settings
        public int bulletsPerBurst = 3;
        public float timeBetweenBursts = 5f;
        public float timeBetweenShotsInBurst = 0.1f;

        // FX prefab
        public string firingFXPrefabName = "FX_Firing";

        // Internal state
        private float burstTimer = 0f;
        private int shotsFiredInBurst = 0;
        private float shotTimer = 0f;
        private bool isBursting = false;

        GameObject telegraph;
        bool telegraphed = false;

        GameObject wings;

        public GameObject CreateBullet(Vector3 startPos, Vector3 angle)
        {
            string prefabPath = "Prefabs/" + projectilePrefabName + ".prefab";
            GameObject newBullet = CreateGameObject(prefabPath);

            Transform t = newBullet.GetComponent<Transform>();
            t.Position = startPos;
            t.Rotation = angle;
            t.Scale = bulletScale;

            Projectile p = newBullet.As<Projectile>();
            p.SetUp();
            p.speed = bulletSpeed;
            p.owner = gameObject;
            p.damage = bulletDamage;
            p.distanceBeforeDestroy = distanceBeforeDestroyBullet;

            allProjectiles.Add(p);

            return newBullet;
        }

        public GameObject CreateTelegraph()
        {
            string prefabPath = "Prefabs/FX_LaserPointer.prefab";
            GameObject fx = CreateGameObject(prefabPath);

            Transform fxTransform = fx.GetComponent<Transform>();
            Transform casterTransform = this.GetComponent<Transform>();

            // Parent it to the caster first
            fx.SetParent(gameObject);

            // Reset local transform (so it sits exactly on the caster)
            fxTransform.Position = Vector3.Zero;
            fxTransform.Rotation = Vector3.Zero;
            fxTransform.Scale = Vector3.One;

            return fx;
        }

        public void CreateFiringFX(Vector3 position, Vector3 rotation)
        {
            string prefabPath = "Prefabs/" + firingFXPrefabName + ".prefab";
            GameObject fx = CreateGameObject(prefabPath);

            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public override void OnCreate()
        {
            base.OnCreate();

            GameObject[] children = gameObject.GetAllChildren();
            foreach (GameObject child in children)
            {
                if (child.tag == "AimingMechWings")
                {
                    wings = child;
                }
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            if (!active)
                return;

            // Always face player
            this.transform.LookAt(
                Bootstrap.Player.transform.Position + new Vector3(0, aimVerticalOffset, 0),
                new Vector3(0, 1, 0)
            );

            if (!isBursting)
            {
                burstTimer += dt;

                if (!telegraphed)
                {
                    CreateTelegraph();
                    telegraphed = true;
                }

                if (burstTimer >= timeBetweenBursts)
                {
                    burstTimer = 0f;
                    isBursting = true;
                    shotsFiredInBurst = 0;
                    shotTimer = 0f;
                }
            }
            else
            {
                shotTimer += dt;

                if (shotTimer >= timeBetweenShotsInBurst)
                {
                    shotTimer = 0f;

                    Transform t = this.GetComponent<Transform>();

                    // Fire bullet
                    CreateBullet(t.WorldPosition, t.WorldRotationQuat.ToEuler());

                    // Spawn firing FX
                    CreateFiringFX(t.WorldPosition, t.WorldRotationQuat.ToEuler());

                    shotsFiredInBurst++;

                    if (shotsFiredInBurst >= bulletsPerBurst)
                    {
                        isBursting = false;
                        telegraphed = false;
                    }
                }
            }

            AimingMechWings amw = wings.As<AimingMechWings>();
            float denom = Utilities.Max(shotTimer + burstTimer, 0.0001f);
            amw.rotateSpeedMultiplier = Utilities.Clamp(5.0f / denom, 0.0f, 10.0f);
        }
    }
}
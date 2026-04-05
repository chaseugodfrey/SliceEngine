using SliceEngine;
using System;
using System.Collections.Generic;
using System.IO;

namespace SliceEngine
{
    public class TurretMech : EnemyBase
    {
        public List<Projectile> allProjectiles = new List<Projectile>();

        // Bullet settings
        public string projectilePrefabName = "Projectile";
        //public float projPerSecond = 10.0f;
        public float bulletSpeed = 500.0f;
        public Vector3 bulletScale = new Vector3(1);
        public int bulletDamage = 1;
        public float distanceBeforeDestroyBullet = 1000.0f;

        // Aim settings
        public float aimVerticalOffset = 1f;
        public float bloomAmount = 0.0f;
        public float minAimRange = 50.0f;
        public float maxAimRange = 200.0f;
        public float predictionStrength = 0.75f;

        // Burst settings
        public int bulletsPerBurst = 3;
        public float timeBetweenBursts = 5f;
        public float timeBetweenShotsInBurst = 0.1f;

        // FX prefab
        public string firingFXPrefabName = "FX_Firing1";

        // Internal state
        float count = 0f;
        float burstTimer = 0f;
        int shotsFiredInBurst = 0;
        float shotTimer = 0f;
        bool isBursting = false;

        GameObject telegraph;
        bool telegraphed = false;

        Transform firingOffset;
        Transform vrot;
        Renderer coreRenderer;

        Vector4 colourActivated;
        Vector4 colourDeactivated = new Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        public GameObject CreateBullet(Vector3 startPos, Vector3 direction)
        {
            string prefabPath = "Prefabs/" + projectilePrefabName + ".prefab";
            GameObject newBullet = CreateGameObject(prefabPath);

            Transform t = newBullet.GetComponent<Transform>();
            t.Position = startPos;
            t.Scale = bulletScale;

            // Apply bloom (spread)
            direction += new Vector3(
                SliceRandom.RangeFloat(-bloomAmount, bloomAmount),
                SliceRandom.RangeFloat(-bloomAmount, bloomAmount),
                0f
            ) * 0.01f;

            // Normalize
            direction = direction.Normalize();

            // Set bullet rotation
            t.Rotation = Quaternion.LookRotation(direction).ToEuler();

            Projectile p = newBullet.As<Projectile>();
            p.SetUp();
            p.speed = bulletSpeed;
            p.owner = gameObject;
            p.damage = bulletDamage;
            p.distanceBeforeDestroy = distanceBeforeDestroyBullet;

            allProjectiles.Add(p);

            return newBullet;
        }

        public void SetTelegraph()
        {
            telegraph.SetActive(telegraphed);
            foreach (GameObject child in telegraph.GetAllChildren())
            {
                child.SetActive(telegraphed);
            }
            return;
        }

        public void CreateFiringFX(Vector3 position, Vector3 rotation)
        {
            string prefabPath = "Prefabs/" + firingFXPrefabName + ".prefab";
            GameObject fx = CreateGameObject(prefabPath);

            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public void CreateDamagedFX(Vector3 position, Vector3 rotation)
        {
            GameObject fx = CreateGameObject("Prefabs/FX_TurretMechDamaged.prefab");
            fx.GetComponent<Transform>().Position = position;

            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public void CreateDeathFX(Vector3 position, Vector3 rotation)
        {
            GameObject fx = CreateGameObject("Prefabs/FX_TurretMechDestroyed.prefab");
            fx.GetComponent<Transform>().Position = position;

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
                if (child.tag == "Telegraph")
                {
                    telegraph = child;
                }
                if (child.tag == "FiringOffset")
                {
                    firingOffset = child.GetComponent<Transform>();
                }
                if (child.tag == "VRot")
                {
                    vrot = child.GetComponent<Transform>();
                }
                if (child.tag == "Core")
                {
                    coreRenderer = child.GetComponent<Renderer>();
                    colourActivated = coreRenderer.GetColor();
                }
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            Vector3 pos = transform.Position;

            if (!active)
                return;

            Vector3 playerPos = Bootstrap.Player.transform.WorldPosition;
            float distanceToPlayer = Utilities.Distance3D(transform.WorldPosition, playerPos);

            if (distanceToPlayer <= maxAimRange && distanceToPlayer >= minAimRange)
            {
                coreRenderer.SetColor(colourActivated);
                coreRenderer.SetEmissionColor(colourActivated);

                Vector3 offset = new Vector3(0, aimVerticalOffset, 0);
                Vector3 origin = firingOffset.WorldPosition;
                Vector3 target = playerPos + offset;
                Vector3 toPlayer = target - origin;
                RayCastHit hit;
                if (Physics.Raycast(origin, toPlayer, out hit, LayerMask.GetCollisionMask("LineOfSight"), QueryTriggerInteraction.Ignore))
                {
                    if (hit.transform.gameObject.tag == "Player")
                    {
                        Vector3 playerVel = Bootstrap.Player.GetComponent<RigidBody>().Velocity * predictionStrength;
                        Vector3 lookTarget = target;
                        Vector3 aimTarget;

                        bool usePrediction = ((shotsFiredInBurst + 1) % 3 != 0);

                        if (usePrediction && playerVel.SquareMagnitude() > 1.0f)
                        {
                            float timeToHit = distanceToPlayer / bulletSpeed;
                            Vector3 predictedPos = playerPos + playerVel * timeToHit;
                            aimTarget = predictedPos + new Vector3(0, aimVerticalOffset, 0);
                        }
                        else
                        {
                            aimTarget = playerPos + new Vector3(0, aimVerticalOffset, 0);
                        }

                        Vector3 shootDir = (aimTarget - transform.WorldPosition).Normalize();

                        Vector3 hTarget = new Vector3(
                            aimTarget.x,
                            transform.WorldPosition.y,
                            aimTarget.z
                        );

                        transform.LookAt(hTarget, new Vector3(0, 1, 0));

                        Vector3 worldDir = aimTarget - vrot.WorldPosition;

                        // Convert to hrot local space
                        Vector3 forward = transform.Forward; // Z axis
                        Vector3 up = transform.Up;           // Y axis

                        // Project onto axes
                        float forwardDot = Vector3.Dot(worldDir, forward);
                        float upDot = Vector3.Dot(worldDir, up);

                        // Calculate pitch (up/down)
                        float pitch = -Utilities.Rad2Deg((float)Math.Atan2(upDot, forwardDot));

                        pitch = Utilities.Clamp(pitch, -60f, 60f);

                        // Apply ONLY X rotation
                        vrot.Rotation = new Vector3(pitch, 0f, 0f);

                        bool shouldTelegraph = burstTimer >= timeBetweenBursts - 0.75f;

                        if (shouldTelegraph != telegraphed)
                        {
                            telegraphed = shouldTelegraph;
                            SetTelegraph();
                        }

                        if (!isBursting)
                        {
                            burstTimer += dt;

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

                                // Fire bullet
                                CreateBullet(transform.WorldPosition, shootDir);

                                // Spawn firing FX
                                CreateFiringFX(firingOffset.GetComponent<Transform>().WorldPosition, transform.WorldRotationQuat.ToEuler());

                                shotsFiredInBurst++;

                                if (shotsFiredInBurst >= bulletsPerBurst)
                                {
                                    isBursting = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        telegraphed = false;
                        SetTelegraph();
                    }
                }
                else
                {                    
                    telegraphed = false;
                    SetTelegraph();
                }
            }
            else
            {
                if (distanceToPlayer < minAimRange)
                {
                    coreRenderer.SetColor(colourDeactivated);
                    coreRenderer.SetEmissionColor(colourDeactivated);
                    vrot.Rotation = new Vector3(15.0f, 0f, 0f);
                }
                telegraphed = false;
                SetTelegraph();
            }
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            SliceLog.Console($"Take Damage called for {amount}");

            base.TakeDamage(amount, source);
        }

        protected override void OnDamaged(GameObject source)
        {
            CreateDamagedFX(transform.WorldPosition, Vector3.Zero);
            base.OnDamaged(source);
        }

        public override void OnDeath()
        {
            CreateDeathFX(transform.WorldPosition, Vector3.Zero);
            base.OnDeath();
        }
    }
}
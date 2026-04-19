using SliceEngine;
using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class AimingMech : SliceBehaviour
    {
        public List<Projectile> allProjectiles = new List<Projectile>();

        // Bullet settings
        public string projectilePrefabName = "Projectile";
        //public float projPerSecond = 10.0f;
        public float bulletSpeed = 100.0f;
        public Vector3 bulletScale = new Vector3(1);
        public int bulletDamage = 1;
        public float distanceBeforeDestroyBullet = 250.0f;

        // Aim settings
        public float aimVerticalOffset = 1f;
        public float bloomAmount = 5.0f;
        public bool active = false;
        public float maxAimRange = 100.0f;
        public float predictionStrength = 0.75f;

        // Burst settings
        public int bulletsPerBurst = 3;
        public float timeBetweenBursts = 5f;
        public float timeBetweenShotsInBurst = 0.1f;
        public float chargeTime = 1.5f;

        // FX prefab
        public string firingFXPrefabName = "FX_Firing1";

        // Follow settings — when followTarget is set the mech tracks a point relative to the target
        public Transform followTarget;
        public Vector3 followOffset;
        public bool useCameraRelativeOffset = false;
        public float followSpeed = 1.5f;
        public float destinationLagSpeed = 0.8f; // how slowly the target destination itself updates — lower = more lag

        private Vector3 smoothedDestination;
        private bool destinationInitialised = false;

        // Internal state
        float count = 0f;
        float burstTimer = 0f;
        int shotsFiredInBurst = 0;
        float shotTimer = 0f;
        bool isBursting = false;

        // Bobbing
        float bobTimer = 0f;
        float bobAmplitude = 1.0f;
        float bobFrequency = 1.5f;
        float baseY = 0f;

        GameObject telegraph;
        bool telegraphed = false;

        GameObject wings;

        //public GameObject fireSFXSourceObject;
        AudioSource fireSFXSource;

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

        public void ResetTelegraph()
        {
            telegraph.SetActive(false);
            foreach (GameObject child in telegraph.GetAllChildren())
            {
                child.SetActive(false);
            }
            return;
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

        public override void OnCreate()
        {
            base.OnCreate();

            baseY = transform.Position.y;

            GameObject[] children = gameObject.GetAllChildren();
            foreach (GameObject child in children)
            {
                if (child.tag == "AimingMechWings")
                {
                    wings = child;
                }
                if (child.tag == "Telegraph")
                {
                    telegraph = child;
                }
            }

            if (gameObject.HasComponent<AudioSource>())
            {
                fireSFXSource = gameObject.GetComponent<AudioSource>();
                fireSFXSource.Volume = 0.5f;
                fireSFXSource.IsLoop = false;
            }
            else
            {
                SliceLog.Log("No Audio Source");
            }

        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            // Follow target position using camera-relative or world-space offset
            if (followTarget != null)
            {
                Vector3 worldOffset = useCameraRelativeOffset ? CameraRelativeOffset(followOffset) : followOffset;
                Vector3 rawDestination = followTarget.WorldPosition + worldOffset;

                // Stage 1: lag the destination itself so camera turns don't snap the mech immediately
                if (!destinationInitialised)
                {
                    smoothedDestination = rawDestination;
                    destinationInitialised = true;
                }
                smoothedDestination = new Vector3(
                    Utilities.Lerp(smoothedDestination.x, rawDestination.x, destinationLagSpeed * dt),
                    Utilities.Lerp(smoothedDestination.y, rawDestination.y, destinationLagSpeed * dt),
                    Utilities.Lerp(smoothedDestination.z, rawDestination.z, destinationLagSpeed * dt)
                );

                // Stage 2: move the mech toward the smoothed destination
                baseY = smoothedDestination.y;

                Vector3 pos = transform.Position;
                pos.x = Utilities.Lerp(pos.x, smoothedDestination.x, followSpeed * dt);
                pos.z = Utilities.Lerp(pos.z, smoothedDestination.z, followSpeed * dt);
                transform.Position = pos;
            }

            // Bobbing motion
            bobTimer += dt;

            Vector3 bobPos = transform.Position;
            bobPos.y = baseY + Utilities.Sin(bobTimer * bobFrequency) * bobAmplitude;

            transform.Position = bobPos;

            if (!active)
                return;

            Vector3 playerPos = Bootstrap.Player.transform.WorldPosition;
            float distanceToPlayer = Utilities.Distance3D(transform.WorldPosition, playerPos);
            if (distanceToPlayer <= maxAimRange)
            {
                Vector3 offset = new Vector3(0, aimVerticalOffset, 0);
                Vector3 origin = transform.WorldPosition;
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

                        this.transform.LookAt(lookTarget, new Vector3(0, 1, 0));

                        bool shouldTelegraph = burstTimer >= timeBetweenBursts - chargeTime;

                        if (shouldTelegraph != telegraphed)
                        {
                            telegraphed = shouldTelegraph;
                            SetTelegraph();

                            SliceLog.Console("ALOY Play Fire SFX");
                        }

                        if(telegraphed)
                        {
                            fireSFXSource.Play();
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
                                CreateFiringFX(transform.WorldPosition, transform.WorldRotationQuat.ToEuler());

                                shotsFiredInBurst++;

                                if (shotsFiredInBurst >= bulletsPerBurst)
                                {
                                    isBursting = false;
                                    telegraphed = false;
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
                telegraphed = false;
                SetTelegraph();
            }

            AimingMechWings amw = wings.As<AimingMechWings>();
            float denom = Utilities.Max(shotTimer + burstTimer, 0.0001f);
            amw.rotateSpeedMultiplier = Utilities.Clamp(5.0f / denom, 0.0f, 10.0f);
        }

        // Rotates a local-space offset into world space using the camera's facing direction
        private Vector3 CameraRelativeOffset(Vector3 localOffset)
        {
            Vector3 forward = Bootstrap.CameraController.transform.Forward;
            forward.y = 0;
            float mag = forward.Magnitude();
            if (mag > 0.001f) forward = forward / mag;

            Vector3 right = new Vector3(forward.z, 0, -forward.x);

            return right * localOffset.x + new Vector3(0, localOffset.y, 0) + forward * localOffset.z;
        }
    }
}

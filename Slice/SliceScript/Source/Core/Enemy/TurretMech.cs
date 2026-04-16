using SliceEngine;
using System;
using System.Collections;
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
        public float aimSpeed = 8f;

        // Burst settings
        public int bulletsPerBurst = 3;
        public float timeBetweenBursts = 5f;
        public float timeBetweenShotsInBurst = 0.1f;
        public float telegraphDuration = 2.3f;

        // FX prefabs
        public string firingFXPrefabName = "FX_Firing1";
        public string telegraphFXPrefabName = "";

        // SFX Objects
        public GameObject ChargeUpAudioObject;
        public GameObject ChargeDownAudioObject;
        float poweringTime = 1f;


        // Internal state
        float count = 0f;
        float burstTimer = 0f;
        int shotsFiredInBurst = 0;
        float shotTimer = 0f;
        bool isBursting = false;
        bool powered = false;
        bool powering = false;

        GameObject telegraph;
        bool telegraphed = false;
        float currentPitch = 0f;
        GameObject telegraphFXInstance;

        Transform firingOffset;
        Transform vrot;
        Renderer coreRenderer;
        AudioSource chargeAudio;

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

            chargeAudio = gameObject.GetComponent<AudioSource>();
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
                if (!powered)
                {
                    powered = true;

                    if (ChargeUpAudioObject.HasComponent<AudioSource>())
                    {
                        SliceLog.Log("HAVE AUDIO COMPONENT");
                        AudioSource a = ChargeUpAudioObject.GetComponent<AudioSource>();
                        a.Play();
                    }

                }

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

                        // Disable prediction during telegraph - track player directly for accurate indicator
                        bool usePrediction = !telegraphed && ((shotsFiredInBurst + 1) % 3 != 0);

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

                        // Horizontal - lerp toward target direction for smooth tracking
                        float lerpT = Utilities.Clamp(aimSpeed * dt, 0f, 1f);
                        Vector3 targetHDir = new Vector3(hTarget.x - transform.WorldPosition.x, 0f, hTarget.z - transform.WorldPosition.z).Normalize();
                        Vector3 currentHDir = new Vector3(transform.Forward.x, 0f, transform.Forward.z).Normalize();
                        Vector3 lerpedHDir = new Vector3(
                            currentHDir.x + (targetHDir.x - currentHDir.x) * lerpT,
                            0f,
                            currentHDir.z + (targetHDir.z - currentHDir.z) * lerpT
                        ).Normalize();
                        transform.LookAt(transform.WorldPosition + lerpedHDir, new Vector3(0, 1, 0));

                        Vector3 worldDir = aimTarget - vrot.WorldPosition;

                        // Convert to hrot local space
                        Vector3 forward = transform.Forward; // Z axis
                        Vector3 up = transform.Up;           // Y axis

                        // Project onto axes
                        float forwardDot = Vector3.Dot(worldDir, forward);
                        float upDot = Vector3.Dot(worldDir, up);

                        // Calculate target pitch (up/down)
                        float targetPitch = -Utilities.Rad2Deg((float)Math.Atan2(upDot, forwardDot));
                        targetPitch = Utilities.Clamp(targetPitch, -60f, 60f);

                        // Vertical - lerp pitch for smooth tracking when player jumps
                        currentPitch += (targetPitch - currentPitch) * lerpT;
                        vrot.Rotation = new Vector3(currentPitch, 0f, 0f);

                        bool shouldTelegraph = burstTimer >= timeBetweenBursts - telegraphDuration;

                        if (shouldTelegraph != telegraphed)
                        {
                            telegraphed = shouldTelegraph;
                            SetTelegraph();

                            if (telegraphed)
                            {
                                chargeAudio.Play();
                                if (telegraphFXPrefabName != "")
                                {
                                    telegraphFXInstance = CreateGameObject("Prefabs/" + telegraphFXPrefabName + ".prefab");
                                    telegraphFXInstance.SetParent(firingOffset.gameObject);
                                    telegraphFXInstance.GetComponent<Transform>().Position = Vector3.Zero;
                                    telegraphFXInstance.GetComponent<Transform>().Rotation = Vector3.Zero;
                                }
                            }
                            else
                            {
                                chargeAudio.Stop();
                                if (telegraphFXInstance != null) { telegraphFXInstance.Destroy(); telegraphFXInstance = null; }
                            }
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
                                AudioSettings.PlaySFX("TurretFire");

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
                        if (telegraphed) { chargeAudio.Stop(); burstTimer = 0f; if (telegraphFXInstance != null) { telegraphFXInstance.Destroy(); telegraphFXInstance = null; } }
                        telegraphed = false;
                        SetTelegraph();
                    }
                }
                else
                {
                    if (telegraphed) { chargeAudio.Stop(); burstTimer = 0f; if (telegraphFXInstance != null) { telegraphFXInstance.Destroy(); telegraphFXInstance = null; } }
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

                    if (powered)
                    {
                        powered = false;

                        if (ChargeDownAudioObject.HasComponent<AudioSource>())
                        {
                            SliceLog.Log("HAVE AUDIO COMPONENT");
                            AudioSource a = ChargeDownAudioObject.GetComponent<AudioSource>();
                            a.Play();
                        }
                    }
                    float lerpT = Utilities.Clamp(1f * dt, 0f, 1f);

                    // Vertical - lerp pitch for smooth tracking when player jumps
                    currentPitch += (15f - currentPitch) * lerpT;
                    vrot.Rotation = new Vector3(currentPitch, 0f, 0f);


                    //vrot.Rotation = new Vector3(15.0f, 0f, 0f);
                }
                if (telegraphed) { chargeAudio.Stop(); if (telegraphFXInstance != null) { telegraphFXInstance.Destroy(); telegraphFXInstance = null; } }
                telegraphed = false;
                SetTelegraph();
            }
        }

        IEnumerator PowerUp()
        {
            powered = false;
            powering = true;

            float poweringCount = 0f;

            if (ChargeUpAudioObject.HasComponent<AudioSource>())
            {
                ChargeUpAudioObject.GetComponent<AudioSource>().Play();
            }

            while (powering)
            {
                poweringCount += Time.deltaTime;
                if (poweringCount >= poweringTime) 
                {
                    powering = false; 
                }

                // Rotate The fellah



                yield return null; // Waits for next Frame
            }

            powering = false;
            powered = true;


            yield break;
        }

        IEnumerator PowerDown() 
        {
            powered = true;
            powering = true;

            SliceLog.Log("Powered is " + powered + " | Powering is " + powering);

            float poweringCount = 0f;

            AudioSource a;

            if (ChargeDownAudioObject.HasComponent<AudioSource>())
            {
                SliceLog.Log("HAVE AUDIO COMPONENT");
                a = ChargeDownAudioObject.GetComponent<AudioSource>();
                a.Play();
                SliceLog.Log( "IS it playing????? ====" + a.IsPlaying);
            }
            else
            {
                SliceLog.Log("NO AUDIO COMPONENT");
            }

                while (powering)
                {
                    
                    poweringCount += Time.deltaTime;
                    if (poweringCount >= poweringTime)
                    {
                        powering = false;
                    }

                    // Rotate The fellah



                    yield return null; // Waits for next Frame
                }

            powering = false;
            powered = false;


            yield break;
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
using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.PerformanceData;
using System.IO.Pipes;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Channels;
using System.Security.Permissions;


namespace SliceEngine
{
    public class Projectile_Spawner: SliceBehaviour
    {
        public List<Projectile> allProjectiles = new List<Projectile>();

        //Bullet controls
        public string projectilePrefabName = "Projectile";
        public float projPerSecond = 4f;
        public float bulletSpeed = 1f;
        public Vector3 bulletScale = new Vector3(1);
        public int bulletDamage = 1;
        public bool projDestroysOnImpact = true;
        public float distanceBeforeDestroyBullet = 10f;

        //Burst Controls
        public bool burstProjectiles = false;
        public int burstRatePerSecond = 10;
        public int burstCount = 3;

        //Circle controls
        public bool circle = false;
        public int bulletsPerCircle = 4;
        public float circleRadius = 1f;

        //Random Radial
        public float radialRandomInDegrees = 0f;

        //Spinning Controls
        public float spiralRate = 1f; // seconds for a rotation
        public Vector3 spiralAxis = new Vector3(0,1,0);

        //Aiming Controls
        public float aimVerticalOffset = 1f;
        public bool preaim = true;
        public float preAimFlickerRate = 10f;
        public float preaimMinAlpha = .3f;
        public float preaimMaxAlpha = 1f;
        public float preAimPercentage = .2f; //percantage of the 
        public bool preaiming = false;
        public bool preAimRandom = false;
        public string aiminglinePrefabName = "PreAim";

        public GameObject preAimObject;

        //Shooting Effects
        public string shootFXPrefabName1 = "FX_Firing1";
        public string shootFXPrefabName2 = "FX_Firing2";
        //
        public bool active = false;

        public int limit = 100;
        public float rangeLimit = 10f;

        public int spawnStyle = 0;
        public enum SpawnStyle { Straight, Spiral, Aim, Nothing };
        public SpawnStyle currentStyle = SpawnStyle.Straight;


        #region bullet creation
        public GameObject CreateBullet(Vector3 startPos, Vector3 angle, Vector3 scale, float speed, bool destroyOnImpact, float distanceBeforeDestroy)
        {
            //if (Bootstrap.Player.transform.WorldPosition.Distance(transform.WorldPosition) < 300.0f)
            //{
            //}

            //SliceLog.Log("Bullet created");

            string prefabPath = "Prefabs/" + projectilePrefabName + ".prefab";
            //GameObject newBullet = CreateGameObject("Prefabs/Projectile.prefab");
            GameObject newBullet = CreateGameObject(prefabPath);            

            AudioSettings.PlaySFX("EnemyProjectile", newBullet);
            Transform tempT = newBullet.GetComponent<Transform>();

            tempT.Position = startPos;
            tempT.Rotation = angle;
            tempT.Scale = scale;            

            Projectile tempP = newBullet.As<Projectile>();

            tempP.SetUp();
            tempP.speed = speed;
            tempP.owner = gameObject;
            tempP.damage = bulletDamage;
            tempP.distanceBeforeDestroy = distanceBeforeDestroy;
            tempP.destroyOnImpact = destroyOnImpact;

            allProjectiles.Add(tempP);

            if (allProjectiles.Count > limit)
            {
                for (int i = 0; i < (allProjectiles.Count - limit); i++)
                {
                    DestroyBullet(allProjectiles[0]);
                }
            }

            if (currentStyle != SpawnStyle.Spiral)
            {
                string fxPrefabPath = "Prefabs/" + shootFXPrefabName1 + ".prefab";

                GameObject firingEffect = CreateGameObject(fxPrefabPath);
                Transform tempT2 = firingEffect.GetComponent<Transform>();
                tempT2.Position = startPos;
            }
            //else
            //{
            //    string fxPrefabPath = "Prefabs/" + shootFXPrefabName2 + ".prefab";

            //    GameObject firingEffect = CreateGameObject(fxPrefabPath);
            //    Transform tempT2 = firingEffect.GetComponent<Transform>();
            //    tempT2.Position = startPos;
            //}

            return newBullet;
        }

        public void DestroyBullet(Projectile toDestroy)
        {
            int index = allProjectiles.IndexOf(toDestroy);

            if (index != -1)
            {
                Projectile temp = allProjectiles[index];
                allProjectiles.RemoveAt(index);
                temp.gameObject.Destroy();
            }
        }

        public void ClearBullets()
        {
            for (int i = allProjectiles.Count -1 ; i <= 0; i--)
            {
                Projectile temp = allProjectiles[i];
                allProjectiles.RemoveAt(i);
                temp.gameObject.Destroy();   
            }
        }

        public void SpawnSetProjectile()
        {
            if (circle)
            {
                SpawnInCircle(bulletsPerCircle, circleRadius);
            }
            else
            {
                Transform T = this.GetComponent<Transform>();

                CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed, projDestroysOnImpact, distanceBeforeDestroyBullet);
            }
        }

        public void SpawnInCircle(int number, float radius)
        {
            

            float degree = 360f / (float)number;

            //SliceLog.Log("Spawn in circle is called for [" + number + "] number of buttlets with [" + radius + "] radius. The calculated degrees is " + degree);

            Transform T = this.GetComponent<Transform>();

            Transform copiedT = new Transform(this.gameObject);

            for (int i = 0; i< number; i++)
            {
                //SliceLog.Log("T pre rotation is at" + T.Rotation);

                T.Rotate(degree, T.Up);

                //SliceLog.Log("T post rotation is at" + T.Rotation);

                CreateBullet(T.WorldPosition, T.Rotation, bulletScale, bulletSpeed, projDestroysOnImpact, distanceBeforeDestroyBullet);
            }

            T.Rotation = copiedT.Rotation;
        }

        public void SpawnInBurstCheck()
        {
            if (HasComponent<AudioSource>())
            {
                GetComponent<AudioSource>().Play();
            }

            if (burstProjectiles)
            {
                StartCoroutine(SpawnInBurstCoroutine());
            }
            else
            {
                SpawnSetProjectile();
            }
        }

        IEnumerator SpawnInBurstCoroutine()
        {
            float count = 0f;
            float rateInSeconds = 1f /  (float) burstRatePerSecond ;
            int bulletsSpawned = 0;
            while (active && bulletsSpawned < burstCount)
            {
                count += Time.fixedDeltaTime;
                if (count >= rateInSeconds)
                {
                    count -= rateInSeconds;
                    SpawnSetProjectile();
                    bulletsSpawned++;
                }

                yield return new WaitForSeconds(Time.fixedDeltaTime);
            }

            yield break;
        }
        #endregion

        public float count = 0f;

        public override void OnCreate()
        {
            base.OnCreate();
            currentStyle = (SpawnStyle)spawnStyle;
            Console.WriteLine("In Projectile Spawner Create");

            if (currentStyle == SpawnStyle.Aim || currentStyle == SpawnStyle.Straight)
            {
                Console.WriteLine($"Aiming line prefab name {aiminglinePrefabName}");
                string aimingPrefabPath = "Prefabs/" + aiminglinePrefabName + ".prefab";
                //GameObject newBullet = CreateGameObject("Prefabs/Projectile.prefab");
                Console.WriteLine($"pre aim {aimingPrefabPath}");
                preAimObject = CreateGameObject(aimingPrefabPath);
                Console.WriteLine("Creating pre aim object");

                preAimObject.SetParent(this.gameObject);

                Console.WriteLine("parenting pre aim object");

                Transform T = preAimObject.GetComponent<Transform>();
                T.Position = new Vector3(0);
                T.Rotation = new Vector3(0);

                //preAim.
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);


            if (!active)
            {
                return;
            }

            count += dt;

            currentStyle = (SpawnStyle)spawnStyle;

            

            switch(currentStyle)
            {
                case SpawnStyle.Spiral:

                    this.transform.Rotate( (360f / spiralRate) * dt , spiralAxis);

                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;

                        
                        SpawnInBurstCheck();
                    }


                    break;
                case SpawnStyle.Aim:

                    if ((this.transform.WorldPosition - Bootstrap.Player.transform.WorldPosition).Magnitude() > rangeLimit)
                    {
                        if (preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            //SliceLog.Log("passed the check on preaim");
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();

                            a.active = false;
                        }

                        if (HasComponent<AudioSource>())
                        {
                            GetComponent<AudioSource>().Play();
                        }

                        break;
                    }


                    this.transform.LookAt(Bootstrap.Player.transform.Position + new Vector3(0, aimVerticalOffset,0), new Vector3(0,1,0));

                    float calc = 1f / projPerSecond;

                    // Find the charging up time
                    if (preaiming == false && count >= (calc * (1f - preAimPercentage)))
                    {
                        preaiming = true;
                        //start Preaiming
                        if (preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            SliceLog.Log("passed the check on preaim");
                            AlphaWiggleAnimation a =  preAimObject.As<AlphaWiggleAnimation>();

                            a.active = true;
                            a.rate = preAimFlickerRate;
                            a.MinWiggle = preaimMinAlpha;
                            a.MaxWiggle = preaimMaxAlpha;
                            a.random = preAimRandom;
                        }
                        //flicker
                    }


                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;

                        if (preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            SliceLog.Log("passed the check on reset");
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();

                            preaiming = false;

                            a.Reset();
                        }

                        SpawnInBurstCheck();
                    }

                    break;
                case SpawnStyle.Straight:

                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;

                        this.transform.LookAt(
                            Bootstrap.Player.transform.Position + new Vector3(0, aimVerticalOffset, 0),
                            new Vector3(0, 1, 0)
                        );

                        // hide the pre-aim line on fire
                        if (preAimObject != null && preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();
                            preaiming = false;
                            a.Reset();
                        }

                        SpawnInBurstCheck();

                        if (HasComponent<AudioSource>())
                        {
                            GetComponent<AudioSource>().Play();
                        }
                    }
                    else if (!preaiming && count >= (1f / projPerSecond) * (1f - preAimPercentage))
                    {
                        // start showing the pre-aim line during charge-up
                        preaiming = true;
                        this.transform.LookAt(
                            Bootstrap.Player.transform.Position + new Vector3(0, aimVerticalOffset, 0),
                            new Vector3(0, 1, 0)
                        );

                        if (preAimObject != null && preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();
                            a.active = true;
                            a.rate = preAimFlickerRate;
                            a.MinWiggle = preaimMinAlpha;
                            a.MaxWiggle = preaimMaxAlpha;
                            a.random = preAimRandom;
                        }
                    }

                    break;
                case SpawnStyle.Nothing:
                    //does nothing for testing purooses
                    break;
            }

        }
    }
}
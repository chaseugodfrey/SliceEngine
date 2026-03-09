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

        //
        public bool circle = false;
        public int bulletsPerCircle = 4;
        public float circleRadius = 1f;

        //Random Radial
        public float radialRandomInDegrees = 0f;

        //Spinning Controls
        public float spiralRate = 1f; // seconds for a rotation
        public Vector3 spiralAxis = new Vector3(0,1,0);

        //
        public bool active = false;

        public int limit = 100;


        public float rangeLimit = 10f;

        public int spawnStyle = 0;
        private enum SpawnStyle { Straight, Spiral, Aim };
        private SpawnStyle currentStyle = SpawnStyle.Straight;


        #region bullet creation
        public GameObject CreateBullet(Vector3 startPos, Vector3 angle, Vector3 scale, float speed, bool destroyOnImpact, float distanceBeforeDestroy)
        {
            string prefabPath = "Prefabs/" + projectilePrefabName + ".prefab";
            //GameObject newBullet = CreateGameObject("Prefabs/Projectile.prefab");
            GameObject newBullet = CreateGameObject(prefabPath);

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

            Transform T = this.GetComponent<Transform>();

            Transform copiedT = new Transform(this.gameObject);

            for (int i = 0; i< number; i++)
            {
                copiedT.Rotate(degree, T.Up);

                CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed, projDestroysOnImpact, distanceBeforeDestroyBullet);
            }
        }

        public void SpawnInBurstCheck()
        {
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

        private float count = 0f;

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
                    break;


                    this.transform.LookAt(Bootstrap.Player.transform.Position, new Vector3(0,1,0));

                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;

                        SpawnInBurstCheck();
                    }

                    break;
                case SpawnStyle.Straight:

                    if (count >= 1 /projPerSecond)
                    {
                        count -= 1 / projPerSecond;

                        SpawnInBurstCheck();
                    }

                    break;
            }

        }
    }
}
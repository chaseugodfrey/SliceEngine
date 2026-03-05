using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class Projectile_Spawner: SliceBehaviour
    {
        public List<Projectile> allProjectiles = new List<Projectile>();

        public float projPerSecond = 4f;

        public float bulletSpeed = 1f;

        public Vector3 bulletScale = new Vector3(1);

        public int bulletDamage = 1;

        public int spawnStyle = 0;

        public float spiralRate = 1f; // seconds for a rotation

        public Vector3 spiralAxis = new Vector3(0,1,0);

        public bool active = false;

        public int limit = 100;

        public float RangeLimit = 10f;

        private enum SpawnStyle { Straight, Spiral, Aim };

        private SpawnStyle currentStyle = SpawnStyle.Straight;

        public GameObject CreateBullet(Vector3 startPos, Vector3 angle, Vector3 scale, float speed)
        {

            GameObject newBullet = CreateGameObject("Prefabs/Projectile.prefab");
            //Console.WriteLine("Creating bullet");
            Transform tempT = newBullet.GetComponent<Transform>();
            //Console.WriteLine($"Bullet transform: {tempT.Position.ToString()}, {tempT.Rotation.ToString()}, {tempT.Scale.ToString()}");
            
            tempT.Position = startPos;
            tempT.Rotation = angle;
            tempT.Scale = scale;
            //Console.WriteLine($"Target transform: {startPos.ToString()}, {angle.ToString()}, {scale.ToString()}");

            //Console.WriteLine($"Bullet transform Part 2: {tempT.Position.ToString()}, {tempT.Rotation.ToString()}, {tempT.Scale.ToString()}");

            Projectile tempP = newBullet.As<Projectile>();

            tempP.SetUp();
            tempP.speed = speed;
            tempP.owner = this;
            tempP.damage = bulletDamage;

            allProjectiles.Add(tempP);

            //Console.WriteLine($"Bullet transform Part 3: {tempT.Position.ToString()}, {tempT.Rotation.ToString()}, {tempT.Scale.ToString()}");

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

        private float count = 0f;

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);


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

                        Transform T = this.GetComponent<Transform>();

                        CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed);
                    }


                    break;
                case SpawnStyle.Aim:

                    if ((this.transform.Position - Bootstrap.Player.transform.Position).Magnitude() > RangeLimit)
                    break;


                    this.transform.LookAt(Bootstrap.Player.transform.Position, new Vector3(0,1,0));

                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;

                        Transform T = this.GetComponent<Transform>();

                        CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed);
                    }

                    break;
                case SpawnStyle.Straight:

                    if (count >= 1 /projPerSecond)
                    {
                        count -= 1 / projPerSecond;

                        Transform T = this.GetComponent<Transform>();

                        CreateBullet(T.WorldPosition, T.WorldRotationQuat.ToEuler(), bulletScale, bulletSpeed);
                    }

                    break;
            }

        }
    }
}
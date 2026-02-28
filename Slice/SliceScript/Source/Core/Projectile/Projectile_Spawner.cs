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

        public float bulletScale = 1f;

        public int spawnStyle = 0;

        public float spiralRate = 1f; // seconds for a rotation

        public Vector3 spiralAxis = new Vector3(0,1,0);

        public bool active = false;

        public int limit = 100;

        private enum SpawnStyle { Straight, Spiral, Fan };

        private SpawnStyle currentStyle = SpawnStyle.Straight;

        public GameObject CreateBullet(Vector3 startPos, Vector3 angle, float scale, float speed)
        {
            GameObject newBullet = CreateGameObject("Prefabs/Projectile.prefab");

            newBullet.GetComponent<Transform>().Position = startPos;

            newBullet.GetComponent<Transform>().Rotation = angle;

            newBullet.GetComponent<Transform>().Scale = new Vector3(scale);

            newBullet.As<Projectile>().speed = speed;

            newBullet.As<Projectile>().owner = this;

            allProjectiles.Add(newBullet.As<Projectile>());

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

                        CreateBullet(T.Position, T.Rotation, bulletScale, bulletSpeed);
                    }


                    break;
                case SpawnStyle.Fan:
                case SpawnStyle.Straight:

                    if (count >= 1 /projPerSecond)
                    {
                        count -= 1 / projPerSecond;

                        Transform T = this.GetComponent<Transform>();

                        CreateBullet(T.Position, T.Rotation, bulletScale ,bulletSpeed);
                    }

                    break;
            }

        }
    }
}
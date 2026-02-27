using SliceEngine;
using System;
using System.Collections.Generic;
using System.Security.Permissions;


namespace SliceEngine
{
    public class Projectile_Spawner: SliceBehaviour
    {
        public List<GameObject> bullets = new List<GameObject>();

        public GameObject CreateBullet(Vector3 startPos, Vector3 angle, float speed)
        {
            GameObject newBullet = CreateGameObject("Prefabs/Bullet.prefab");


            return newBullet;
        }
    }
}
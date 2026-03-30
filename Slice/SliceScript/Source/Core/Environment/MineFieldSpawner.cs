using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class MineFieldSpawner : SliceBehaviour
    {
        public int mineCount = 12;
        public float spawnRadius = 8f;
        public float minSpawnRadius = 2f;
        public string minePrefabPath = "Prefabs/LandmineMech.prefab";

        public float launchSpeedMin = 5f;
        public float launchSpeedMax = 10f;
        public float launchUpwardForce = 8f;
        public float timeBetweenMines = 0.08f; // delay between each mine spawn

        private bool triggered = false;

        public override void OnTriggerEnter(uint other)
        {
            if (triggered) return;

            base.OnTriggerEnter(other);

            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO == null || collidedGO.tag != "Player") return;

            triggered = true;
            StartCoroutine(SpawnMines());
        }

        private IEnumerator SpawnMines()
        {
            Vector3 origin = transform.WorldPosition;

            for (int i = 0; i < mineCount; i++)
            {
                float angle = SliceRandom.RangeFloat(0f, 360f) * ((float)Math.PI / 180f);
                float distance = SliceRandom.RangeFloat(minSpawnRadius, spawnRadius);

                float offsetX = (float)Math.Cos(angle) * distance;
                float offsetZ = (float)Math.Sin(angle) * distance;

                GameObject mine = CreateGameObject(minePrefabPath);
                Transform t = mine.GetComponent<Transform>();
                t.Position = origin;
                t.Rotation = new Vector3(0f, SliceRandom.RangeFloat(0f, 360f), 0f);
                t.Scale = new Vector3(1f, 1f, 1f);

                Vector3 outward = new Vector3(offsetX, 0f, offsetZ).Normalize();
                float lateralSpeed = SliceRandom.RangeFloat(launchSpeedMin, launchSpeedMax);

                Vector3 launchVelocity = new Vector3(
                    outward.x * lateralSpeed,
                    launchUpwardForce,
                    outward.z * lateralSpeed
                );

                RigidBody rb = mine.GetComponent<RigidBody>();
                rb.Velocity = launchVelocity;

                yield return new WaitForSeconds(timeBetweenMines);
            }

            gameObject.Destroy();
        }
    }
}
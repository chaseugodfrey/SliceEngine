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
        public string minePrefabPath = "Prefabs/Landmine.prefab";

        public float launchSpeedMin = 5f;
        public float launchSpeedMax = 10f;
        public float launchUpwardForce = 15f;

        private bool triggered = false;

        public override void OnTriggerEnter(uint other)
        {
            if (triggered) return;

            base.OnTriggerEnter(other);

            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO == null || collidedGO.tag != "Player") return;

            triggered = true;
            SpawnMines();
            gameObject.Destroy();
        }

        private void SpawnMines()
        {
            Vector3 origin = transform.WorldPosition;

            for (int i = 0; i < mineCount; i++)
            {
                float angle = SliceRandom.RangeFloat(0f, 360f) * ((float)Math.PI / 180f);
                float distance = SliceRandom.RangeFloat(minSpawnRadius, spawnRadius);

                float offsetX = (float)Math.Cos(angle) * distance;
                float offsetZ = (float)Math.Sin(angle) * distance;

                Vector3 spawnPos = new Vector3(
                    origin.x + offsetX,
                    origin.y,
                    origin.z + offsetZ
                );

                GameObject mine = CreateGameObject(minePrefabPath);
                Transform t = mine.GetComponent<Transform>();
                t.Position = origin; // spawn at origin, velocity carries them outward
                t.Rotation = new Vector3(0f, SliceRandom.RangeFloat(0f, 360f), 0f);
                t.Scale = new Vector3(1f, 1f, 1f);

                // Outward direction from origin to target landing spot
                Vector3 outward = new Vector3(offsetX, 0f, offsetZ).Normalize();
                float lateralSpeed = SliceRandom.RangeFloat(launchSpeedMin, launchSpeedMax);
                float longitudeSpeed = SliceRandom.RangeFloat(2f, 10f);

                Vector3 launchVelocity = new Vector3(
                    outward.x * lateralSpeed,
                    longitudeSpeed,
                    outward.z * lateralSpeed
                );

                RigidBody rb = mine.GetComponent<RigidBody>();
                rb.Velocity = launchVelocity;
            }
        }
    }
}
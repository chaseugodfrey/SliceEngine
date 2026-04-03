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
        public string minePrefabPath = "Prefabs/Landmine.prefab";

        public float launchSpeedMin = 5f;
        public float launchSpeedMax = 10f;
        public float launchUpwardForce = 8f;
        public float timeBetweenMines = 0.2f; // delay between each mine spawn
        public float lingerTime = 2f;

        private bool triggered = false;

        public override void OnTriggerEnter(uint other)
        {
            if (triggered) return;

            base.OnTriggerEnter(other);

            lingerTime = mineCount * timeBetweenMines * 3;
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO == null || collidedGO.tag != "Player") return;

            triggered = true;
            
            StartCoroutine(WaitForSpawn());
            
        }

        private IEnumerator WaitForSpawn()
        {
            Coroutine test = StartCoroutine(SpawnMines());
            yield return new WaitForCoroutine(test);
            StartCoroutine(Suicide(lingerTime));
        }

        private IEnumerator SpawnMines()
        {
            Vector3 origin = transform.WorldPosition;

            for (int i = 0; i < mineCount; i++)
            {
                float angle = SliceRandom.RangeFloat(0f, 360f) * ((float)Math.PI / 180f);

                float offsetX = (float)Math.Cos(angle);
                float offsetZ = (float)Math.Sin(angle);

                GameObject mine = CreateGameObject(minePrefabPath);
                Transform t = mine.GetComponent<Transform>();
                t.Position = origin;
                t.Rotation = Vector3.Zero;//new Vector3(0f, SliceRandom.RangeFloat(0f, 360f), 0f);
                t.Scale = new Vector3(1f, 1f, 1f);

                Vector3 outward = new Vector3(offsetX, 0f, offsetZ).Normalize();
                float lateralSpeed = SliceRandom.RangeFloat(launchSpeedMin, launchSpeedMax); // determines the distance and speed

                Vector3 launchVelocity = new Vector3(
                    outward.x * lateralSpeed,
                    launchUpwardForce,       // can randomize if i want
                    outward.z * lateralSpeed
                );

                RigidBody rb = mine.GetComponent<RigidBody>();
                rb.Velocity = launchVelocity;

                yield return new WaitForSeconds(timeBetweenMines);
            }

            //gameObject.Destroy();
        }

        IEnumerator Suicide(float time)
        {
            yield return new WaitForSeconds(time);
            gameObject.Destroy();
        }
    }
}
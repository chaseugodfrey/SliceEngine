using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class MineFieldSpawner : SliceBehaviour
    {
        public int mineCount = 8;
        public string minePrefabPath = "LandmineMech";

        public float launchSpeedMin = 10f;
        public float launchSpeedMax = 15f;
        public float launchUpwardForce = 20f;
        public float timeBetweenMines = 0.2f; // delay between each mine spawn

        private bool triggered = false;

        float timeBetweenTriggers = 0.0f;
        public float durationBetweenTriggers = 5.0f;

        public float mineSelfDestructDuration = 5.0f;

        Vector4 colourActivated;
        Vector4 colourDeactivated = new Vector4(0.92f, 0.47f, 0.21f, 1f);

        Renderer coreRenderer;

        public override void OnCreate()
        {
            base.OnCreate();

            GameObject[] children = gameObject.GetAllChildren();
            foreach (GameObject child in children)
            {
                if (child.tag == "Core")
                {
                    coreRenderer = child.GetComponent<Renderer>();
                    colourActivated = coreRenderer.GetColor();
                    coreRenderer.SetColor(colourDeactivated);
                    coreRenderer.SetEmissionColor(colourDeactivated);
                }
            }
        }

        public override void OnFixedUpdate(float dt)
        { 
            if (triggered)
            {
                timeBetweenTriggers += dt;
                coreRenderer.SetColor(colourActivated);
                coreRenderer.SetEmissionColor(colourActivated);
            }

            if (timeBetweenTriggers >= durationBetweenTriggers)
            {
                timeBetweenTriggers = 0.0f;
                triggered = false;
                coreRenderer.SetColor(colourDeactivated);
                coreRenderer.SetEmissionColor(colourDeactivated);
            }
        }

        public override void OnTriggerStay(uint other)
        {
            if (triggered) return;

            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO == null || collidedGO.tag != "Player") return;

            base.OnTriggerEnter(other);

            triggered = true;
            
            StartCoroutine(WaitForSpawn());
            
        }

        private IEnumerator WaitForSpawn()
        {
            Coroutine test = StartCoroutine(SpawnMines());
            yield return new WaitForCoroutine(test);
        }

        private IEnumerator SpawnMines()
        {
            Vector3 origin = transform.WorldPosition;

            for (int i = 0; i < mineCount; i++)
            {
                float angle = SliceRandom.RangeFloat(0f, 360f) * ((float)Math.PI / 180f);

                float offsetX = (float)Math.Cos(angle);
                float offsetZ = (float)Math.Sin(angle);

                GameObject mine = CreateGameObject("Prefabs/" + minePrefabPath + ".prefab");
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

                LandmineMech lm = mine.As<LandmineMech>();
                lm.selfDestruct = true;
                lm.selfDestructTime = mineSelfDestructDuration;

                yield return new WaitForSeconds(timeBetweenMines);
            }

        }
    }
}
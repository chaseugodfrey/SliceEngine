using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Remoting.Metadata.W3cXsd2001;
using System.Security.Cryptography;

namespace SliceEngine
{
    public class AirmineMech : SliceBehaviour
    {
        bool triggered = false;

        public Vector3 startOffset = new Vector3(0, -1.0f, 0.0f);
        public Vector3 endOffset = new Vector3(0, 1.0f, 0.0f);

        public float hoverSpeed = 2.0f;
        public float hoverTimer = 0.0f;
        Vector3 basePosition;

        public bool selfDestruct = false;
        public float selfDestructTime = 30.0f;
        GameObject explosion;
        public override void OnCreate()
        {
            base.OnCreate();

            basePosition = transform.WorldPosition;

            GameObject[] children = gameObject.GetAllChildren();
            foreach (GameObject child in children)
            {
                if (child.tag == "LandmineExplosionCore")
                {
                    explosion = child;
                }
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            if (triggered)
            {
                return;
            }

            if (selfDestruct)
            {
                selfDestructTime -= dt;
                if (selfDestructTime <= 0.0f)
                {
                    if (!triggered)
                    {
                        triggered = true;
                        explosion.As<AirmineMechExplosion>().Triggered();
                        StartCoroutine(Suicide());
                        return;
                    }
                }
            }

            hoverTimer += dt * hoverSpeed;

            float t = Utilities.PingPong(hoverTimer, 1.0f);

            // Interpolate between offsets
            Vector3 offset = Vector3.Lerp(startOffset, endOffset, t);

            transform.Translate(offset * dt);      
        }

        public override void OnTriggerEnter(uint other)
        {
            if (triggered) { return; }
            triggered = true;
            base.OnTriggerEnter(other);
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null && collidedGO.tag == "Player")
            {
                explosion.As<AirmineMechExplosion>().Triggered();
                StartCoroutine(Suicide());
            }
        }

        IEnumerator Suicide()
        {
            yield return new WaitForSeconds(0.5f);
            gameObject.Destroy();
        }
    }
}
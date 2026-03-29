using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class AirmineMech : SliceBehaviour
    {
        bool triggered = false;

        GameObject explosion;
        public override void OnCreate()
        {
            base.OnCreate();
            GameObject[] children = gameObject.GetAllChildren();
            foreach (GameObject child in children)
            {
                if (child.tag == "LandmineExplosionCore")
                {
                    explosion = child;
                }
            }
        }

        public override void OnTriggerEnter(uint other)
        {
            if (triggered) { return; }
            triggered = true;
            base.OnTriggerEnter(other);
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null && collidedGO.tag == "Player")
            {
                explosion.As<LandmineMechExplosion>().Triggered();
                StartCoroutine(Suicide());
            }
        }

        IEnumerator Suicide()
        {
            yield return new WaitForSeconds(5.0f);
            gameObject.Destroy();
        }
    }
}
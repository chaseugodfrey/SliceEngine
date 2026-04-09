using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class LandmineMech : SliceBehaviour
    {
        bool triggered = false;

        GameObject explosion;
        GameObject trigger;

        public bool selfDestruct = false;
        public float selfDestructTime = 5.0f;

        public void ManuallyDetonate()
        {
            trigger.As<LandmineMechTrigger>().triggered = true;
        }
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
                else if (child.tag == "LandmineTrigger")
                {
                    trigger = child;
                }
            }
        }

        public override void OnFixedUpdate(float dt)
        {            
            if (triggered)
            {
                return;
            }
            base.OnFixedUpdate(dt);

            if (selfDestruct)
            {
                selfDestructTime -= dt;
            }

            if ((selfDestruct && selfDestructTime <= 0.0f) || trigger.As<LandmineMechTrigger>().triggered)
            {
                triggered = true;

                AudioSettings.PlaySFX("Explode");
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
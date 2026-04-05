using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO.Pipes;
using System.Security.Permissions;


namespace SliceEngine
{
    public class RotatingEnvironment : SliceBehaviour
    {
        public bool active = false;
        public float spiralRate = 1f; // seconds for a rotation
        public Vector3 spiralAxis = new Vector3(0, 1, 0);

        float currentSpiralRate;

        public override void OnCreate()
        {
            base.OnCreate();
        }

        public override void OnAwake()
        {
            currentSpiralRate = spiralRate;
        }

        public override void OnFixedUpdate(float dt)
        {
            if (!active) return; 

            base.OnFixedUpdate(dt);

            //push it forward based on speed

            if (currentSpiralRate <= 0f)
                return;

            this.transform.Rotate((360f / currentSpiralRate) * dt, spiralAxis);

            //this.GetComponent<Transform>().Position += this.GetComponent<Transform>().Forward.Normalize() * speed * dt; 
        }

        // starts or stops rotating, depending on the value of toBeActive, over the course of timer seconds
        public void StartOrStopRotating(bool toStart, float seconds)
        {
            StartCoroutine(Ramp(toStart, seconds));
        }

        IEnumerator Ramp(bool toBeActive, float timer)
        {
            if (toBeActive)
            {
                currentSpiralRate = 0.01f;
                active = true;
            }

            float elapsedTime = timer;
            while (elapsedTime >= 0.0f)
            {
                float rate = toBeActive ? 1f - elapsedTime / timer : elapsedTime / timer;
                if (rate > 0.0f)
                {
                    rate = 1.0f / rate;
                }

                currentSpiralRate = rate * spiralRate;
                elapsedTime -= Time.deltaTime;
                yield return null;
            }

            if (!toBeActive)
            {
                currentSpiralRate = 0.0f;
                active = false;
            }

        }
    }
}
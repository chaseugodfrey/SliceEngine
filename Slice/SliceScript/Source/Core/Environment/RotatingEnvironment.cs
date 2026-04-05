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
            if (active)
            {
                base.OnFixedUpdate(dt);

                this.transform.Rotate((360f / currentSpiralRate) * dt, spiralAxis);
            }
        }

        // starts or stops rotating, depending on the value of toBeActive, over the course of timer seconds
        public void StartOrStopRotating(bool toStart, float seconds)
        {
            active = toStart;
        }
    }
}
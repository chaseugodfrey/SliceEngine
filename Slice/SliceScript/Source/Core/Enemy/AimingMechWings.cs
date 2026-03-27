using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;


namespace SliceEngine
{
    public class AimingMechWings : SliceBehaviour
    {
        public float rotateSpeed = 100.0f;
        public float rotateSpeedMultiplier = 1.0f;
        private Vector3 dir = new Vector3(1, 0, 0);

        public override void OnFixedUpdate(float dt)
        {
            if (rotateSpeedMultiplier > 0.0f)
            {
                transform.Rotate(rotateSpeed * rotateSpeedMultiplier * dt, dir);
            }
        }
    }
}
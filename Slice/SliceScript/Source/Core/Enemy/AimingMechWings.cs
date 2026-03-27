using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;


namespace SliceEngine
{
    public class AimingMechWings : SliceBehaviour
    {
        Projectile_Spawner ps;
        public float rotateSpeed = 50.0f;

        public override void OnCreate()
        {
            base.OnCreate();
        }

        public override void OnFixedUpdate(float dt)
        {

        }
    }
}
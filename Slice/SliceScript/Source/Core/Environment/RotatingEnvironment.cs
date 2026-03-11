using SliceEngine;
using System;
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

        public override void OnCreate()
        {
            base.OnCreate();
            
        }

        public override void OnFixedUpdate(float dt)
        {
            if (!active) return; 

            base.OnFixedUpdate(dt);

            //push it forward based on speed

            this.transform.Rotate((360f / spiralRate) * dt, spiralAxis);

            //this.GetComponent<Transform>().Position += this.GetComponent<Transform>().Forward.Normalize() * speed * dt; 
        }
    }
}
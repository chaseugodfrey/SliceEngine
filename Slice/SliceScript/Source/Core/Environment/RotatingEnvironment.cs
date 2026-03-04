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

        public override void OnCreate()
        {
            base.OnCreate();
            
        }

        public override void OnUpdate(float dt)
        {
            if (!active) return; 

            base.OnUpdate(dt);

            //push it forward based on speed

            this.transform.Rotate((360f / spiralRate) * dt, transform.Up);

            //this.GetComponent<Transform>().Position += this.GetComponent<Transform>().Forward.Normalize() * speed * dt; 
        }
    }
}
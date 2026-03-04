using SliceEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO.Pipes;
using System.Security.Permissions;


namespace SliceEngine
{
    public class MovingEnvironment : SliceBehaviour
    {
        public bool active = false;
        public float speed = 1.0f;

        public Vector3 position1  = new Vector3(0);
        public Vector3 position2 = new Vector3(0);



        private Vector3 differenceHalfed = new Vector3(0, 0, 0);
        private Vector3 middlePos = new Vector3(0, 0, 0);

        private float cycle = 0f;

        public override void OnCreate()
        {
            base.OnCreate();
            
            differenceHalfed = (position1 - position2) / 2;

            middlePos = position2 + differenceHalfed;
        }

        public override void OnUpdate(float dt)
        {
            if (!active) return 

            base.OnUpdate(dt);

            //push it forward based on speed

            cycle += dt;

            float calc = (float)(1f * Math.Sin(speed * cycle));

            transform.Position = middlePos + (differenceHalfed * calc);

            //this.GetComponent<Transform>().Position += this.GetComponent<Transform>().Forward.Normalize() * speed * dt; 
        }
    }
}
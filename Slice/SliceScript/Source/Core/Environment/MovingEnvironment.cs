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
        public float wait = 3f;

        public Vector3 position1  = new Vector3(0);
        public Vector3 position2 = new Vector3(0);


        private RigidBody rb;
        private Vector3 differenceHalfed = new Vector3(0, 0, 0);
        private Vector3 middlePos = new Vector3(0, 0, 0);

        private float cycle = 0f;
        private bool waiting = false;

        public override void OnCreate()
        {
            base.OnCreate();
            
            differenceHalfed = (position1 - position2) / 2;

            middlePos = position2 + differenceHalfed;

            rb = this.GetComponent<RigidBody>();
        }


        //IEnumerator Wait(float time)
        //{
        //    waiting = true;
        //    float count = 0f;

        //    while(count < time)
        //    {
        //        count += Time.fixedDeltaTime;
        //        yield return new WaitForSeconds(Time.fixedDeltaTime);
        //    }
        //    waiting = false;
        //    yield break;
        //}

        public override void OnFixedUpdate(float dt)
        {
            if (!active || waiting) return;

            base.OnFixedUpdate(dt);

            //push it forward based on speed

            cycle += dt;

            float calc = (float)(Math.Sin(speed * cycle));

            Vector3 targetPos = middlePos + (differenceHalfed * calc);

            // Calculate movement delta
            Vector3 delta = targetPos - transform.Position;

            // Move relative instead of snapping
            transform.Translate(delta); 
        }
    }
}
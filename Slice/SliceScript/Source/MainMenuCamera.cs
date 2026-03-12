using SliceEngine;
using System;


namespace SliceEngine
{
    public class MainMenuCamera : SliceBehaviour
    {
        private Transform lightPos;
        private Transform fanBladeRotation;
        public float minValue = 0f;
        public float maxValue = 0f;
        public float speed = 2.0f;
        public float currAngle = 0f;

        private float timeAccumulator = 0f;

        public override void OnCreate()
        {
            GameObject lightObj = FindGameObjectWithName("MovingLight");
            GameObject fanBladeObj = FindGameObjectWithName("FanBlades");


            if(lightObj != null)
            {
                lightPos = lightObj.GetComponent<Transform>();
            }

            if (fanBladeObj != null)
            {
                fanBladeRotation = fanBladeObj.GetComponent<Transform>();
            }

        }

        public override void OnUpdate(float dt)
        {
            if (lightPos == null)
            {
                return;
            }

            timeAccumulator += dt * speed;

            float distance = maxValue - minValue;

            float bounce = Utilities.PingPong(timeAccumulator, distance);

            Vector3 pos = lightPos.Position;
            pos.y = minValue + bounce;
            lightPos.Position = pos;
            if(fanBladeRotation != null)
            {
                

                fanBladeRotation.Rotate(60.0f*dt, new Vector3(1.0f,0.0f,0.0f)); 

            }
        }
    }
}
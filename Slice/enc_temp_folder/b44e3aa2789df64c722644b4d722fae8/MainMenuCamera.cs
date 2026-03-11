using SliceEngine;
using System;


namespace SliceEngine
{
    public class MainMenuCamera : SliceBehaviour
    {
        private Transform lightPos;
        public float minValue = 0f;
        public float maxValue = 0f;
        public float speed = 2.0f;

        private float timeAccumulator = 0f;

        public override void OnCreate()
        {
            GameObject lightObj = FindGameObjectWithName("MovingLight");


            if(lightObj != null)
            {
                lightPos = lightObj.GetComponent<Transform>();
            }
        
        }

        public override void OnUpdate(float dt)
        {
            if (lightPos == null) return;

            
            timeAccumulator += dt * speed;

            
            float distance = maxValue - minValue;

            
            float pingPongValue = Utilities.PingPong(timeAccumulator, distance);

            
            Vector3 pos = lightPos.Position;
            pos.x = minValue + pingPongValue;
            lightPos.Position = pos;
        }
    }
}
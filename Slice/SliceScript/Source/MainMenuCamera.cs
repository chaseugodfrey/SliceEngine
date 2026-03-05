using SliceEngine;
using System;


namespace SliceEngine
{
    public class MainMenuCamera : SliceBehaviour
    {
        Transform camTrans;
        Vector3 Value;
        GameObject rainEmitter;
        Transform rain;

        public override void OnCreate()
        {
            camTrans = GetComponent<Transform>();
            rainEmitter = FindGameObjectWithName("Cube_1");
            rain = rainEmitter.GetComponent<Transform>();


            Value = new Vector3(0, 0, 1);
        
        }

        public override void OnUpdate(float dt)
        {

            camTrans.Position += Value * dt;
            rain.Position += Value * dt;
        }
    }
}
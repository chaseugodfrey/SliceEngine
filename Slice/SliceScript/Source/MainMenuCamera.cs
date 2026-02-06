using SliceEngine;
using System;


namespace SliceEngine
{
    public class MainMenuCamera : SliceBehaviour
    {
        Transform camTrans;
        public Vector3 Value;

        public override void OnCreate()
        {
            camTrans = GetComponent<Transform>();

            Value = new Vector3(0, 0, 1);
        
        }

        public override void OnUpdate(float dt)
        {

            camTrans.Position += Value * dt;
        }
    }
}
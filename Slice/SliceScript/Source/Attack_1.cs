using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class Attack_1 : SliceBehaviour
    {

        public Vector3 DirectionAndMagnitude;
        public GameObject testsubject;

        public override void OnCreate()
        {
            DirectionAndMagnitude = new Vector3(0, 0, 8);
        }

        public override void OnUpdate(float dt)
        {
            Physics.RayUpdateMovement(gameObject.mID, DirectionAndMagnitude);
        }
    }

}

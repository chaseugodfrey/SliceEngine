using SliceEngine;
using System;


namespace SliceEngine
{
    public class Payload : SliceBehaviour
    {
        public WaypointSystem waypointSystem;

        public override void OnCreate()
        {
            waypointSystem = new WaypointSystem();
        }

        public override void OnUpdate(float dt)
        {
            Vector3 dir = (waypointSystem.GetTargetPosition() - GetComponent<Transform>().Position).Normalize();

        }
    }
}

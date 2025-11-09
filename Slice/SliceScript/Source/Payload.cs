using SliceEngine;
using System;


namespace SliceEngine
{
    public class Payload : SliceBehaviour
    {
        public WaypointSystem waypointSystem;

        public float moveSpeed = 3.0f;
        // TODO: Show arrays in inspector like unity does wiht List<> and stuff
        // TODO: GameObject variable should show as a string in inspector, but cna be converted to a gO in script
        public string Waypoint1 = "Waypoint1";
        public string Waypoint2 = "Waypoint2";
        public string Waypoint3 = "Waypoint3";
        public string Waypoint4 = "Waypoint4";

        public override void OnCreate()
        {
            waypointSystem = new WaypointSystem();

            // find a btr way :pepepray:
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint1).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint2).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint3).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint4).GetComponent<Transform>().Position);
        }

        public override void OnUpdate(float dt)
        {
            waypointSystem.Update(GetComponent<Transform>().Position);


            Vector3 dir = (waypointSystem.GetTargetPosition() - GetComponent<Transform>().Position).Normalize();

            GetComponent<Transform>().Position += dir * moveSpeed * dt;

        }
    }
}

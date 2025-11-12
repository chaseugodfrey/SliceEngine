using SliceEngine;
using System;


namespace SliceEngine
{
    public class Payload : SliceBehaviour
    {
        public WaypointSystem waypointSystem;
        public int[] test2;
        public float moveSpeed = 3.0f;
        // TODO: Show arrays in inspector like unity does wiht List<> and stuff
        // TODO: GameObject variable should show as a string in inspector, but cna be converted to a gO in script
        public string Waypoint1 = "Waypoint1";
        public string Waypoint2 = "Waypoint2";
        public string Waypoint3 = "Waypoint3";
        public string Waypoint4 = "Waypoint4";
        public string Waypoint5 = "Waypoint5";
        public string Waypoint6 = "Waypoint6";
        public string Waypoint7 = "Waypoint7";
        public string Waypoint8 = "Waypoint8";
        public string Waypoint9 = "Waypoint9";
        public string Waypoint10 = "Waypoint10";

        public override void OnCreate()
        {
            waypointSystem = new WaypointSystem();

            // find a btr way :pepepray:
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint1).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint2).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint3).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint4).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint5).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint6).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint7).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint8).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint9).GetComponent<Transform>().Position);
            waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(Waypoint10).GetComponent<Transform>().Position);
        }

        public override void OnUpdate(float dt)
        {
            waypointSystem.Update(GetComponent<Transform>().Position);


            Vector3 dir = (waypointSystem.GetTargetPosition() - GetComponent<Transform>().Position).Normalize();

            GetComponent<Transform>().Position += dir * moveSpeed * dt;

        }
    }
}

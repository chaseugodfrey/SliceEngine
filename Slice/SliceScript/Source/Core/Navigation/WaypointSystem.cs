using System;
using System.Collections.Generic;

namespace SliceEngine
{ 
    public class WaypointSystem
    {
        public List<Vector3> Waypoints;
        public int currPoint = 0;


        public void Update(Vector3 Position)
        {
            if (currPoint < Waypoints.Count)
            {
                if (Position.Distance(Waypoints[currPoint]) < 1.0f)
                {
                    // if its close enough
                    currPoint++;
                }
            }
            else
            {
                // for now just set it back ig
                // im not sure whats should the intended behaviour be
                currPoint = 0;
            }
        }

        public Vector3 GetTargetPosition()
        {
            if (currPoint <  Waypoints.Count)
                return Waypoints[currPoint];

            return Vector3.Zero;
        }
    }
}

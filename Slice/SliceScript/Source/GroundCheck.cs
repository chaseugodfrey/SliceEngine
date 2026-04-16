using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class GroundCheck : SliceBehaviour
    {
        static int count = 0;

        public string groundName = "Ground Check";
        private GameObject[] groundObject;
        public bool grounded;

        private List<GameObject> movingPlatforms = new List<GameObject>();

        private int TriggerBoxCount = 0;
        

        public override void OnCreate()
        {
            base.OnCreate();

            groundObject = gameObject.FindGameObjectsWithTag(groundName);
            //if (groundObject != null) Console.WriteLine("Found ground");
            //else Console.WriteLine("Ground not found");
        }
        public override void OnTriggerEnter(uint other)
        {
            GameObject obj = gameObject.FindGameObjectWithID(other);
            //SliceLog.Console("Collided with " + obj.tag);
            if (obj.layer != LayerMask.NameTolayer("MovingPlatform")) return;
            SliceLog.Console("Adding with " + obj.tag);

            if (movingPlatforms.Contains(obj)) return;
            movingPlatforms.Add(obj);
        }

        public override void OnTriggerExit(uint other)
        {
            GameObject obj = gameObject.FindGameObjectWithID(other);
            if (obj.layer != LayerMask.NameTolayer("MovingPlatform")) return;
            SliceLog.Console("Stop collision with " + obj.tag);
            if (movingPlatforms.Contains(obj))
            {
                movingPlatforms.Remove(obj);
                Bootstrap.Player.AddVelocityModifier(Vector3.Zero);

            }
        }

        public override void OnFixedUpdate(float dt)
        {
            if (movingPlatforms.Count == 0) return;

            Ray ray = new Ray(Vector3.Down, transform.WorldPosition);

            if (Physics.Raycast(ray, out RayCastHit hit))
            {
                if (hit.transform.gameObject.mID == movingPlatforms[0].mID)
                {
                    SliceLog.Console("Velocity " + movingPlatforms[0].GetComponent<RigidBody>().Velocity);
                    Bootstrap.Player.AddVelocityModifier(movingPlatforms[0].GetComponent<RigidBody>().Velocity);
                }
            }
        }
    }
}
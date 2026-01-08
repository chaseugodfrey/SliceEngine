using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class GroundCheck : SliceBehaviour
    {
        public string groundName = "Ground";
        private GameObject[] groundObject;
        private bool grounded;
        public bool Grounded { get { return grounded; } }
        public override void OnCreate()
        {
            base.OnCreate();

            groundObject = gameObject.FindGameObjectsWithTag(groundName);
            if (groundObject != null) Console.WriteLine("Found ground");
            else Console.WriteLine("Ground not found");
        }
        public override void OnTriggerEnter(uint other)
        {
            if (IsGround(other))
            {
                Console.WriteLine("Player grounded");
                Bootstrap.Player.OnGrounded();
                grounded = true;
                Console.WriteLine("Grounded set to true");
            }
        }
        public override void OnTriggerExit(uint other)
        {
            if (IsGround(other))
            {
                Console.WriteLine("Player off ground");
                grounded = false;
                Console.WriteLine("Grounded set to false");
            }
        }
        private bool IsGround(uint id)
        {
            foreach (GameObject ground in groundObject)
            {
                if (id == ground.mID)
                    return true;
            }
            return false;
        }
    }
}

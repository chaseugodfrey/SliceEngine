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
        private GameObject groundObject; 
        private bool grounded;
        public bool Grounded { get { return grounded; } }
        public override void OnCreate()
        {
            base.OnCreate();

            groundObject = gameObject.FindGameObjectWithName(groundName);
            if (groundObject != null) Console.WriteLine("Found ground id: " + groundObject.mID);
            else Console.WriteLine("Ground not found");
        }
        public override void OnTriggerEnter(uint other)
        {
            if (other == groundObject.mID)
            {
                Console.WriteLine("Player grounded");
                Bootstrap.Player.OnGrounded();
                grounded = true;
            }
        }
        public override void OnTriggerExit(uint other)
        {
            if (other == groundObject.mID)
            {
                Console.WriteLine("Player off ground");
                grounded = false;
            }
        }
    }
}

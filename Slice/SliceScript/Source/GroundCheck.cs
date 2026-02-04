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

        private int TriggerBoxCount = 0;

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

                TriggerBoxCount += 1;
                Console.WriteLine("Grounded set to true");
            }
        }
        public override void OnTriggerExit(uint other)
        {
            if (IsGround(other))
            {
                TriggerBoxCount -= 1;
                TriggerBoxCount = Utilities.Clamp<int>(TriggerBoxCount, 0, 999);
                Console.WriteLine("Player off ground");
                if (TriggerBoxCount <= 0)
                {
                    grounded = false;
                }
                Console.WriteLine("Grounded set to false");
            }
        }
        private bool IsGround(uint id)
        {
            if (gameObject.FindGameObjectWithID(id).tag == groundName)
            { 
                    return true;
            }
            return false;
        }
    }
}

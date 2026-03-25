using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class GroundCheck : SliceBehaviour
    {
        public string groundName = "Ground Check";
        private GameObject[] groundObject;
        public bool grounded;

        private int TriggerBoxCount = 0;

        public override void OnCreate()
        {
            base.OnCreate();

            groundObject = gameObject.FindGameObjectsWithTag(groundName);
            //if (groundObject != null) Console.WriteLine("Found ground");
            //else Console.WriteLine("Ground not found");
        }

    }
}
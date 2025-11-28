using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestMisc : SliceBehaviour
    {
        public override void OnCreate()
        {
            RigidBody rb = GetComponent<RigidBody>();
            if (rb == null)
            {
                SliceLog.Log("Get Component Returned Null Successfully :)");
            }
            else
            {
                SliceLog.Log("Get Component Returned Something Else :(");
            }
        }
    }
}

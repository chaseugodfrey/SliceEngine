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
            var objs = FindGameObjectsWithTag("A Tag That Will Never Exist");
            if (objs.Length == 0)
            {
                SliceLog.Log("FindGameObjectsWithTag in TestMisc script successfully returned null");
            }

            var obj = FindGameObjectWithName("A Name That Will Never Exist");
            if (obj is null)
            {
                SliceLog.Log("FindGameObjectWithName in TestMisc script successfully returned null");
            }

        }
    }
}

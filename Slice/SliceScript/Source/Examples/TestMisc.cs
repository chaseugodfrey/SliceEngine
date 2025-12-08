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
            //var objs = FindGameObjectsWithTag("A Tag That Will Never Exist");
            //if (objs is null)
            //{
            //    SliceLog.Log("FindGameObjectsWithTag in TestMisc script successfully returned null");
            //}

            //var obj = FindGameObjectWithName("A Name That Will Never Exist");
            //if (obj is null)
            //{
            //    SliceLog.Log("FindGameObjectWithName in TestMisc script successfully returned null");
            //}

            //Heresy?
            FindGameObjectWithName("A Name That Will Never Exist")?.GetComponent<RigidBody>()?.AddForce(Vector3.Zero);
            FindGameObjectsWithTag("A Tag That Will Never Exist")?[0]?.GetComponent<RigidBody>()?.AddForce(Vector3.Zero);
        }
    }
}

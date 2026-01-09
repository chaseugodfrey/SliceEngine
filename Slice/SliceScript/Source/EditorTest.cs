using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EditorTest : SliceBehaviour
    {
        public bool boolTest = false;
        public GameObject GOtest;
        public Vector3 vec3test;
        public override void OnUpdate(float dt)
        {

            GOtest = FindGameObjectWithName("Cube");

            SliceLog.Log("X: " + vec3test.x + " Y: "+ vec3test.y + " Z: " + vec3test.z);
        }
    }
}

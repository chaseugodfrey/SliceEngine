using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestMovement : SliceBehaviour
    {
        public float speed = 5.0f;
        public Vector3 direction = new Vector3(-1.0f, 0.0f, 0.0f);

        public TestEntity te;
        

        public override void OnCreate()
        {
            te = GetComponent<TestEntity>();

            //transform = GetComponent<Transform>();
        }

        public override void OnUpdate(float dt)
        {
            //transform.Position = new Vector3(0, 0, 0);
        }

    }
}

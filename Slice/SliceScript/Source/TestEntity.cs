using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestEntity : SliceBehaviour
    {
        public float speed = 5.0f;
        public Vector3 direction = new Vector3(1.0f, 0.0f, 0.0f);

        public override void OnCreate()
        {
            Console.WriteLine("TestEntity Created with ID: " + mID);
        }

        public override void OnUpdate(float dt)
        {
            //Console.WriteLine("TestEntity Update called with dt: " + dt);

            if (Input.IsKeyPressed(Keys.KEY_W) || Input.IsKeyDown(Keys.KEY_W))
            {
                Console.WriteLine("TestEntity Update called with dt: " + dt);
            }
        }

    }
}

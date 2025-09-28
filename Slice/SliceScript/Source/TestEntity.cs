using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestEntity : SliceBehaviour
    {
        public override void OnCreate()
        {
            Console.WriteLine("TestEntity Created with ID: " + mID);
        }

        public override void OnUdate(float dt)
        {
            Console.WriteLine("TestEntity Update called with dt: " + dt);
        }

    }
}

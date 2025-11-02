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

        public Transform t;
        

        public override void OnCreate()
        {
            Console.WriteLine("Test Movement onCreate");
            t = GetComponent<Transform>();           
        }

        public override void OnUpdate(float dt)
        {
            // This part is not even running
            Console.WriteLine("I'm running btw");
            t.Position -= direction * speed * dt;
        }

    }
}

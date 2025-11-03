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

        Transform t;

        public override void OnCreate()
        {
            t = GetComponent<Transform>();
        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_1) || Input.IsKeyDown(Keys.KEY_1))
            {
                Console.WriteLine("IM MOVING");
                t.Position += direction * speed * dt;
            }

            if (Input.IsKeyPressed(Keys.KEY_2) || Input.IsKeyDown(Keys.KEY_2))
            {
                // degrees per second
                float rotationSpeed = 90.0f;
                t.Rotation += new Vector3(0.0f, rotationSpeed * dt, 0.0f);
            }

            if (Input.IsKeyPressed(Keys.KEY_3) || Input.IsKeyDown(Keys.KEY_3))
            {
                float scaleSpeed = 1.0f;
                t.Scale += new Vector3(scaleSpeed, scaleSpeed, scaleSpeed) * dt;
            }
        }

    }
}

using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{

    public class MovingPlatform : SliceBehaviour
    {
        public float moveSpeed = 1.0f;
        public Vector3 direction;
        RigidBody rb;

        public override void OnCreate()
        {
            rb = gameObject.GetComponent<RigidBody>();
        }

        public override void OnFixedUpdate(float dt)
        {
            rb.Velocity = direction * moveSpeed;
        }
    }
}

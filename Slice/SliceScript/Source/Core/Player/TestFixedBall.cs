using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestFixedBall : SliceBehaviour
    {
        public delegate void Attack();
        public Attack OnAttack;

        private RigidBody rb;

        public override void OnAwake()
        {
            base.OnAwake();
            rb = this.GetComponent<RigidBody>();
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);
            Testing();
        }

        public void Testing()
        {
            //SliceLog.Log("I AM BEING PUSHEDDD");
            rb.AddForce(new Vector3(100, 0, 0) * Time.fixedDeltaTime, ForceMode.Impulse);
        }

    }
}

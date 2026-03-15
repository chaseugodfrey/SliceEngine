using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class IntroController : SliceBehaviour
    {
        public override void OnCreate()
        {
            Bootstrap.CameraController.LockCamera = true;
            Bootstrap.Player.PlayerMovementState = PlayerController.MovementState.Falling;
            Bootstrap.Player.SetPlayerLock(true);
        }

        public override void OnLateUpdate(float dt)
        {
            if (Bootstrap.Player.GetComponent<RigidBody>().Velocity.y < -60.0f)
            {
                Vector3 vel = Bootstrap.Player.GetComponent<RigidBody>().Velocity;
                vel.y = -60.0f;
                Bootstrap.Player.GetComponent<RigidBody>().Velocity = vel;
            }
        }

    }
}

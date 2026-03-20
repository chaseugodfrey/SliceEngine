using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestInputCam : SliceBehaviour
    {
        Quaternion actual_cam_quat;
        Vector3 sensitivity = new Vector3(10f, 10f, 0f);

        public override void OnAwake()
        {
            actual_cam_quat = transform.RotationQuat;
        }
        public override void OnUpdate(float dt)
        {
            Vector2 mouseDelta = Input.GetMouseDelta();
            //SliceLog.Log(mouseDelta.ToString());
            float yawDelta = mouseDelta.x * sensitivity.x * dt;
            float pitchDelta = mouseDelta.y * sensitivity.y * dt;

            //apply yaw rotation
            Quaternion yaw_quat = Quaternion.FromAxisAngle(Vector3.Up, yawDelta);
            actual_cam_quat = (yaw_quat * actual_cam_quat).Normalize();
            //apply pitch rotation
            //float clampedPitch = Utilities.Clamp(pitch + pitchDelta, yClamp.x, yClamp.y);
            //Quaternion pitch_quat = Quaternion.FromAxisAngle(Vector3.Right, clampedPitch - pitch);
            //actual_cam_quat = (actual_cam_quat * pitch_quat).Normalize();
            //pitch = clampedPitch;
            //apply camera shake
            //set camera's rotation
            transform.RotationQuat = actual_cam_quat;
            //  Console.Write("helloooo");

        }
    }
}
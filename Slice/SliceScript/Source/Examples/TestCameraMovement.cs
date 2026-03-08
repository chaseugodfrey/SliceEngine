using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestCameraMovement : SliceBehaviour
    {
        public float testMagnitude = 0.3f;
        public float testDuration = 0.5f;
        public override void OnCollideEnter(uint other)
        {

            //Bootstrap.CameraController.InitiateCameraMovement();
            Bootstrap.CameraController.Shake(testDuration, testMagnitude);
        }
    }
}

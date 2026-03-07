using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestCameraMovement : SliceBehaviour
    {
        public override void OnCollideEnter(uint other)
        {
            
            Bootstrap.CameraController.InitiateCameraMovement();
        }
    }
}

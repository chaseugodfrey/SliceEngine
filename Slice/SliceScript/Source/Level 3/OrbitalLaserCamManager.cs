using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class OrbitalLaserCamManager : SliceBehaviour
    {
        bool isBeingHit = false;
        bool isHit = false;

        public GameObject camObj;
        public Camera cam;

        public override void OnCreate()
        {
            base.OnCreate();
            cam = camObj.GetComponent<Camera>();
        }

        public override void OnLateUpdate(float dt)
        {
            base.OnLateUpdate(dt);
        }

        public void Hit()
        {
            isBeingHit = true;
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Camera : Component
    {
        public Camera() { }
        public Camera(GameObject entity)
        {
            gameObject = entity;
        }

        public static void SetMainCamera(GameObject go)
        {
            FunctionCalls.Camera_SetMainCamera(go.mID);
        }

        public void SetImpactFrame(bool enable)
        {
            FunctionCalls.Camera_ToggleImpactFrames(gameObject.mID, enable);
        }
        public void SetImpactFramePosition(Vector3 pos)
        {
            FunctionCalls.Camera_SetImpactFrameWorldPosition(gameObject.mID, ref pos);
        }
    }
}

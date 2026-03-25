using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Camera
    {
        public static void SetMainCamera(GameObject go)
        {
            FunctionCalls.Camera_SetMainCamera(go.mID);
        }

        public void SetImpactFrame(GameObject go, bool enable)
        {
            FunctionCalls.Camera_ToggleImpactFrames(go.mID, enable);
        }
        public void SetImpactFramePosition(GameObject go, Vector3 pos)
        {
            FunctionCalls.Camera_SetImpactFrameWorldPosition(go.mID, ref pos);
        }
    }
}

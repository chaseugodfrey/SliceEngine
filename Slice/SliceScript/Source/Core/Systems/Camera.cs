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

        public static float Exposure
        {
            get { return FunctionCalls.Camera_GetGamma(); }
            set { FunctionCalls.Camera_SetGamma(value); }
        }
    }
}

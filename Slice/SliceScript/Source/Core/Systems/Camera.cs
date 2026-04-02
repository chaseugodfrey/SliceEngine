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
        public static float Gamma
        {
            get { return FunctionCalls.Camera_GetGamma(); }
            set { FunctionCalls.Camera_SetGamma(value); }
        }
        public float FOV
        {
            get { return FunctionCalls.Camera_GetFOV(gameObject.mID); }
            set { FunctionCalls.Camera_SetFOV(gameObject.mID, value); }
        }
        public void SetImpactFrame(bool enable)
        {
            FunctionCalls.Camera_ToggleImpactFrames(gameObject.mID, enable);
        }
        public void SetImpactFramePosition(Vector3 pos)
        {
            FunctionCalls.Camera_SetImpactFrameWorldPosition(gameObject.mID, ref pos);
        }
        public void SetImpactFrameColor1(Vector3 col)
        {
            FunctionCalls.Camera_SetImpactFrameColor1(gameObject.mID, ref col);
        }
        public void SetImpactFrameColor2(Vector3 col)
        {
            FunctionCalls.Camera_SetImpactFrameColor2(gameObject.mID, ref col);
        }
        public void SetImpactFrameIsSmooth(bool isSmooth)
        {
            FunctionCalls.Camera_SetImpactFrameSmooth(gameObject.mID, isSmooth);
        }
        public void SetImpactFrameSpeed(float speed)
        {
            FunctionCalls.Camera_SetImpactFrameSpeed(gameObject.mID, speed);
        }
        public void SetImpactFrameSharpness(float sharpness)
        {
            FunctionCalls.Camera_SetImpactFrameSharpness(gameObject.mID, sharpness);
        }
        public void SetImpactFrameDensity(float density)
        {
            FunctionCalls.Camera_SetImpactFrameDensity(gameObject.mID, density);
        }
        // in range: [0.f, 1.f]
        public void SetImpactBlend(float blend)
        {
            FunctionCalls.Camera_SetImpactBlend(gameObject.mID, blend);
        }
    }
}

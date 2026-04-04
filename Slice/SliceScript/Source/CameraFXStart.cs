using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class CameraFXStart : SliceBehaviour
    {
        public bool cameraShake = false;
        public bool impactFrame = false;

        public float delay; 
        public float duration;
        public float shakeIntensity;

        public override void OnCreate()
        {
            StartCoroutine(StartFXRoutine());
        }
        private IEnumerator StartFXRoutine()
        {
            yield return new WaitForSeconds(delay);

            if (cameraShake) Bootstrap.CameraController.Shake(duration, shakeIntensity);
            if (impactFrame)
            {
                Camera cam = Bootstrap.CameraController.cam;
                cam.SetImpactFrame(true);
                cam.SetImpactFramePosition(new Vector3(0, 490, 0));
                cam.SetImpactFrameColor1(new Vector3(1, 1, 1));
                cam.SetImpactFrameColor2(new Vector3(0, 0, 0));
                cam.SetImpactFrameIsSmooth(true);
                cam.SetImpactFrameSpeed(-100);
                cam.SetImpactFrameSharpness(200);
                cam.SetImpactFrameDensity(21);

                yield return new WaitForSeconds(duration);

                cam.SetImpactFrame(false);
            }
            }
        }
}

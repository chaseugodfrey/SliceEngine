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
        public float cameraShakeDuration = 5;
        public float shakeIntensity;

        GameObject camObj;
        Vector3 shakeOffset;
        GameObject[] bossGameObjects;

        float loadTimerMax = 12.0f;
        float loadTimer;
        bool isSFXPlaying = false;

        public override void OnCreate()
        {
            StartCoroutine(StartFXRoutine());
            loadTimer = loadTimerMax;
        }

        public override void OnUpdate(float dt)
        {
            loadTimer -= dt;
            if (loadTimer <= loadTimerMax - 4.5f)
            {
                if (!isSFXPlaying)
                {
                    GetComponent<AudioSource>().Play();
                    isSFXPlaying = true;
                }

                else
                {
                    GetComponent<AudioSource>().Volume = loadTimer / (loadTimerMax - 4.5f);
                }
            }

            if (loadTimer <= 1)
            {
                SceneManager.LoadWithoutTransition("Credits");
            }

        }
        private IEnumerator StartFXRoutine()
        {
            camObj = gameObject.FindGameObjectsWithTag("Camera")[0];
            GameObject[] camHolderObjs = gameObject.FindGameObjectsWithTag("mainCam");
            bossGameObjects = gameObject.FindGameObjectsWithTag("Boss");
            
            Camera cam = camHolderObjs[0].GetComponent<Camera>();

            Vector3 initialPos = camObj.GetComponent<Transform>().WorldPosition;
            float elapsedTime = 0.0f;

            if (cameraShake)
            {
                StartCoroutine(ShakeSequence(delay, shakeIntensity * 0.2f, initialPos));
            }

            while (elapsedTime < delay)
            {
                elapsedTime += Time.deltaTime;
                yield return null;
            }

            if (cameraShake)
            {
                StartCoroutine(ShakeSequence(cameraShakeDuration, shakeIntensity, initialPos));
            }

            foreach (GameObject obj in bossGameObjects)
            {
                obj.GetComponent<Transform>().Position = Vector3.Zero;
            }

            if (impactFrame)
            {
                cam.SetImpactFrame(true);
                cam.SetImpactFramePosition(new Vector3(0, 404, 0));
                cam.SetImpactFrameColor1(new Vector3(1, 1, 1));
                cam.SetImpactFrameColor2(new Vector3(0, 0, 0));
                cam.SetImpactFrameIsSmooth(true);
                cam.SetImpactFrameSpeed(-100);
                cam.SetImpactFrameSharpness(200);
                cam.SetImpactFrameDensity(21);
            }

            elapsedTime = 0.0f;

            while (elapsedTime < duration)
            {
                elapsedTime += Time.deltaTime;
                yield return null;
            }

            cam.SetImpactFrame(false);

        }
        private IEnumerator ShakeSequence(float duration, float magnitude, Vector3 initialPos)
        {
            float elapsed = 0f;

            Transform tr = camObj.GetComponent<Transform>(); 
            
            while (elapsed < duration)
            {
                //ideally use perlin noise but this shld be fine for now
                float x = SliceRandom.RangeFloat(-1.0f, 1.0f) * magnitude;
                float y = SliceRandom.RangeFloat(-1.0f, 1.0f) * magnitude;
                float z = SliceRandom.RangeFloat(-1.0f, 1.0f) * magnitude;

                shakeOffset = new Vector3(x, y, z);
                tr.Position = initialPos + shakeOffset;
                elapsed += Time.deltaTime;
                yield return null;
            }

            // Reset offset when finished
            tr.Position = initialPos;
            shakeOffset = Vector3.Zero;
        }
    }
}

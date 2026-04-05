using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Threading;


namespace SliceEngine
{
    public class Lvl3CutsceneManager : SliceBehaviour
    {
        public bool isActive;
        public GameObject CameraRigObj;
        public GameObject CameraObj;
        public GameObject a_camPivot1;
        public GameObject b_camPivot2;

        public float z_transitionDurationToDeath = 1.5f;

        Transform camRigTr;
        Transform bossTr;
        List<Transform> camTransforms;

        Quaternion camInitialRot;
        Vector3 camInitialPos;
        int currentCamIndex;

        public float camLerpTimeMax;
        public float camLerpTimer;

        bool trackBoss;
        bool cutToCamBool;
        bool cutFromCamBool;

        Coroutine fadeInOut;

        public override void OnAwake()
        {
            //camCutsceneTr = a_camCutscene.GetComponent<Transform>();
            camRigTr = CameraRigObj.GetComponent<Transform>();
            camTransforms = new List<Transform>
            {
                a_camPivot1.GetComponent<Transform>(),
                b_camPivot2.GetComponent<Transform>(),
            };
            currentCamIndex = 0;

            bossTr = gameObject.FindGameObjectsWithTag("Boss")[0].GetComponent<Transform>();
            trackBoss = true;
        }

        public void CutToCam(int index, float time)
        {
            if (index >= camTransforms.Count)
            {
                SliceLog.Console("Wrong Index");
                return;
            }

            SliceLog.Console("Cutting to cam " + index);

            // Reset Cameras
            //camCutsceneTr.RotationQuat = camRigTr.RotationQuat;
            //camCutsceneTr.Position = camRigTr.WorldPosition;
            //camCutsceneTr.gameObject.SetActive(true);
            //Camera.SetMainCamera(a_camCutscene);

            // Set cam initial pos
            camInitialRot = camRigTr.WorldRotationQuat;
            camInitialPos = camRigTr.Position;

            // set current cam
            currentCamIndex = index;
            
            // set timer stuff
            isActive = true;
            cutToCamBool = true;
            camLerpTimer = 0.0f;
            camLerpTimeMax = time;
            
        }

        public void StopCutscene()
        {
            cutFromCamBool = true;
        }

        public override void OnUpdate(float dt)
        {
            // first cam tracks boss
            if (trackBoss)
            {
                camTransforms[0].Position = bossTr.WorldPosition + bossTr.Forward * 20.0f;
                camTransforms[0].LookAt(bossTr.WorldPosition, Vector3.Up);
            }

            if (isActive)
            {
                if (camLerpTimer < camLerpTimeMax)
                {
                    camLerpTimer += dt;
                    float rate = camLerpTimer / camLerpTimeMax;

                    if (cutToCamBool)
                    {
                        camRigTr.RotationQuat = Quaternion.Slerp(camInitialRot, camTransforms[currentCamIndex].WorldRotationQuat, rate);
                        camRigTr.Position = Vector3.Lerp(camInitialPos, camTransforms[currentCamIndex].WorldPosition, rate);
                    }

                    else if (cutFromCamBool)
                    {
                        camRigTr.RotationQuat = Quaternion.Slerp(camInitialRot, camTransforms[currentCamIndex].WorldRotationQuat, rate);
                        camRigTr.Position = Vector3.Lerp(camInitialPos, camTransforms[currentCamIndex].WorldPosition, rate);
                    }
                }

                else
                {
                    isActive = false;
                    cutToCamBool = false;
                    cutFromCamBool = false;
                }
            }
        }

        public IEnumerator DeathFadeInOut()
        {
            fadeInOut = StartCoroutine(FadeOutRoutine());
            while (fadeInOut != null)
            {
                yield return null;
            }

            CameraRigObj.As<CameraController>().LockCamera = true;

            camRigTr.transform.Position = camTransforms[0].WorldPosition;
            camRigTr.transform.Rotation = camTransforms[0].Rotation;

            fadeInOut = StartCoroutine(FadeInRoutine());
            while (fadeInOut != null)
            {
                yield return null;
            }
        }

        public IEnumerator FadeInRoutine()
        {
            float elapsedTime = 0f;
            if (SceneManager._transitionRenderer != null)
            {
                SceneManager._transitionRenderer.SetEnabled(true);
            }
            SetRectAlpha(1.0f); // Start black

            while (elapsedTime < z_transitionDurationToDeath)
            {
                elapsedTime += Time.deltaTime;
                float t = Utilities.InverseLerp(0, z_transitionDurationToDeath, elapsedTime);
                float alpha = Utilities.Lerp(1.0f, 0.0f, t); // 1 -> 0

                SetRectAlpha(alpha);
                yield return null;
            }

            Console.Write("End of fade in routine");
            SetRectAlpha(0.0f); // Ensure fully transparent

            if (SceneManager._transitionRenderer != null)
            {
                SceneManager._transitionRenderer.SetEnabled(false);
            }

            fadeInOut = null;
        }

        public IEnumerator FadeOutRoutine()
        {
            float elapsedTime = 0f;
            if (SceneManager._transitionRenderer != null) SceneManager._transitionRenderer.SetEnabled(true);
            SetRectAlpha(0.0f); // Start transparent

            while (elapsedTime < z_transitionDurationToDeath)
            {
                elapsedTime += Time.deltaTime;
                float t = Utilities.InverseLerp(0, z_transitionDurationToDeath, elapsedTime);
                float alpha = Utilities.Lerp(0.0f, 1.0f, t); // 0 -> 1
                                                             //Console.WriteLine("Stuck in fade out routine");
                SetRectAlpha(alpha);
                yield return null;
            }
            SetRectAlpha(1.0f); // Ensure fully black



            fadeInOut = null;
        }

        private void SetRectAlpha(float alpha)
        {
            if (SceneManager._transitionRenderer != null)
            {

                Vector4 color = SceneManager._transitionRenderer.Colour;
                color.w = alpha;
                SceneManager._transitionRenderer.Colour = color;
            }
        }
    }
}
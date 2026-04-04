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
        public GameObject a_camCutscene;
        public GameObject b_camPivot1;

        Transform camCutsceneTr;
        Transform camRigTr;
        List<Transform> camTransforms;

        Quaternion camInitialRot;
        Vector3 camInitialPos;
        int currentCamIndex;

        public float camLerpTimeMax;
        public float camLerpTimer;

        public override void OnAwake()
        {
            camCutsceneTr = a_camCutscene.GetComponent<Transform>();
            camRigTr = CameraRigObj.GetComponent<Transform>();
            camTransforms = new List<Transform>();
            camTransforms.Add(b_camPivot1.GetComponent<Transform>());
            currentCamIndex = 0;
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
            camCutsceneTr.RotationQuat = camRigTr.RotationQuat;
            camCutsceneTr.Position = camRigTr.WorldPosition;
            camCutsceneTr.gameObject.SetActive(true);
            Camera.SetMainCamera(a_camCutscene);

            // Set cam initial pos
            camInitialRot = camCutsceneTr.WorldRotationQuat;
            camInitialPos = camCutsceneTr.Position;

            // set current cam
            currentCamIndex = index;
            
            // set timer stuff
            isActive = true;
            camLerpTimer = 0.0f;
            camLerpTimeMax = time;
            
        }

        public void StopCutscene()
        {

        }

        public override void OnUpdate(float dt)
        {
            if (isActive)
            {
                if (camLerpTimer < camLerpTimeMax)
                {
                    camLerpTimer += dt;

                    float rate = camLerpTimer / camLerpTimeMax;
                    camCutsceneTr.RotationQuat = Quaternion.Slerp(camInitialRot, camTransforms[currentCamIndex].WorldRotationQuat, rate);
                    camCutsceneTr.Position = Vector3.Lerp(camInitialPos, camTransforms[currentCamIndex].WorldPosition, rate);
                }

                else
                {
                    isActive = false;
                }
            }
        }
    }
}
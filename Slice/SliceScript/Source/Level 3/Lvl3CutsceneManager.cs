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

        Transform camRigTr;
        Transform bossTr;
        List<Transform> camTransforms;

        Quaternion camInitialRot;
        Vector3 camInitialPos;
        int currentCamIndex;

        public float camLerpTimeMax;
        public float camLerpTimer;

        bool cutToCamBool;
        bool cutFromCamBool;

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
            camTransforms[0].Position = bossTr.WorldPosition + bossTr.Forward * 20.0f;
            camTransforms[0].LookAt(bossTr.WorldPosition, Vector3.Up);

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
    }
}
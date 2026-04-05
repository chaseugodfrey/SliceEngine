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
        public GameObject BossObj;
        public GameObject CameraRigObj;
        public GameObject CameraObj;
        public GameObject ArenaObj;
        public GameObject ArenaPivotObj;
        public GameObject TriggerBoxObj;
        public GameObject a_camPivot1;
        public GameObject b_camPivot2;
        public GameObject c_camPivot3;

        public GameObject x_BossHud;
        public GameObject x_PlayerHud;

        public float z_transitionDurationToDeath = 1.5f;

        Transform camRigTr;
        Transform camTr;
        Transform bossTr;
        List<Transform> camTransforms;

        Quaternion camRigInitialRot;
        Vector3 camRigInitialPos;
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
            camTr = CameraObj.GetComponent<Transform>();
            camTransforms = new List<Transform>
            {
                a_camPivot1.GetComponent<Transform>(),
                b_camPivot2.GetComponent<Transform>(),
                c_camPivot3.GetComponent<Transform>()
            };
            currentCamIndex = 0;

            bossTr = BossObj.GetComponent<Transform>();
            trackBoss = true;

            StartCoroutine(IntroCutScene());
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
            camRigInitialRot = camRigTr.WorldRotationQuat;
            camRigInitialPos = camRigTr.Position;

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
                        camRigTr.RotationQuat = Quaternion.Slerp(camRigInitialRot, camTransforms[currentCamIndex].WorldRotationQuat, rate);
                        camRigTr.Position = Vector3.Lerp(camRigInitialPos, camTransforms[currentCamIndex].WorldPosition, rate);
                    }

                    else if (cutFromCamBool)
                    {
                        camRigTr.RotationQuat = Quaternion.Slerp(camRigInitialRot, camTransforms[currentCamIndex].WorldRotationQuat, rate);
                        camRigTr.Position = Vector3.Lerp(camRigInitialPos, camTransforms[currentCamIndex].WorldPosition, rate);
                    }
                }

                else
                {
                    camRigTr.RotationQuat = camTransforms[currentCamIndex].WorldRotationQuat;
                    camRigTr.Position = camTransforms[currentCamIndex].WorldPosition;

                    isActive = false;
                    cutToCamBool = false;
                    cutFromCamBool = false;
                }
            }
        }

        public IEnumerator IntroCutScene()
        {
            // cam

            // transforms
            Transform ArenaTr = ArenaObj.GetComponent<Transform>();
            Transform ArenaPivotTr = ArenaPivotObj.GetComponent<Transform>();
            Transform BossTr = BossObj.GetComponent<Transform>();
            Transform PlayerTr = Bootstrap.Player.GetComponent<Transform>();

            // positions
            Vector3 BossInitialPos = BossTr.WorldPosition;
            Vector3 ArenaInitialPos = ArenaTr.WorldPosition;
            Vector3 CamRigInitialPos = ArenaPivotTr.WorldPosition;
            Vector3 PlayerInitialPos = PlayerTr.WorldPosition;

            Vector3 ArenaNewPos = new Vector3(ArenaInitialPos.x, ArenaInitialPos.y - 150.0f, ArenaInitialPos.z);
            Vector3 CamRigNewPos = new Vector3(CamRigInitialPos.x, CamRigInitialPos.y - 150.0f, CamRigInitialPos.z);

            camRigInitialPos = camRigTr.WorldPosition;

            float maxTime = 6.0f;
            float elapsedTime = 0.0f;

            // hide objects
            BossTr.Position = Vector3.Zero;
            Bootstrap.CameraController.LockCamera = true;
            Bootstrap.Player.SetPlayerLock(true);

            camInitialPos = camTr.Position;
            
            // rise up
            while (elapsedTime < maxTime)
            {
                float t = elapsedTime / maxTime;
                float rate = Utilities.SmoothStep(0.0f, 1.0f, t);

                camRigTr.Position = ArenaPivotTr.WorldPosition;
                camRigTr.Rotation = ArenaPivotTr.Rotation;

                ArenaTr.Position = Vector3.Lerp(ArenaNewPos, ArenaInitialPos, rate);
                PlayerTr.Position = Vector3.Zero;
                //Vector3 PlayerNewPos = new Vector3(PlayerInitialPos.x, ArenaTr.Position.y, PlayerInitialPos.x);
                //PlayerTr.Position = PlayerNewPos;
                elapsedTime += Time.deltaTime;
                yield return null;
            }

            elapsedTime = 0.0f;
            maxTime = 0.1f;
            Bootstrap.Player.SetActive(true);
            PlayerTr.Position = PlayerInitialPos;
            BossTr.Position = BossInitialPos;
            camRigTr.Position = camRigInitialPos;
            camTr.Position = camInitialPos;
            Bootstrap.CameraController.LockCamera = false;
            Bootstrap.Player.SetPlayerLock(false);

            while (elapsedTime < maxTime)
            {
                elapsedTime += Time.deltaTime;
                yield return null;
            }

            TriggerBoxObj.SetActive(true);
            BossObj.As<Level3Boss>().StartBoss();
        }

        public IEnumerator DeathFadeInOut(Transform boss)
        {
            if (SceneManager._transitionRenderer != null)
            {
                SceneManager._transitionRenderer.SetEnabled(true);
            }

            float elapsedTime = 0.0f;

            // fade in
            while (elapsedTime < z_transitionDurationToDeath)
            {
                SetRectAlpha(elapsedTime / z_transitionDurationToDeath);
                elapsedTime += Time.deltaTime;
                yield return null;
            }

            // while black
            
            elapsedTime = 0.0f;
            SetRectAlpha(1.0f);

            CutToCam(2, 0);
            CameraRigObj.As<CameraController>().LockCamera = true;
            Bootstrap.Player.SetPlayerLock(true);

            x_BossHud.SetActive(false);
            x_PlayerHud.SetActive(false);

            while (elapsedTime < z_transitionDurationToDeath * 0.5f)
            {
                elapsedTime += Time.deltaTime;
                camRigTr.LookAt(boss.transform.WorldPosition, Vector3.Up);
                yield return null;
            }

            elapsedTime = 0.0f;

            // fade out

            while (elapsedTime < z_transitionDurationToDeath)
            {
                elapsedTime += Time.deltaTime;
                camRigTr.LookAt(boss.transform.WorldPosition, Vector3.Up);
                SetRectAlpha(1.0f - (elapsedTime / z_transitionDurationToDeath));
                yield return null;
            }

            elapsedTime = 0.0f;
            SetRectAlpha(0.0f);
        }

        public IEnumerator FadeOutRoutine()
        {
            float elapsedTime = 0f;
            SetRectAlpha(0.0f); // Start transparent

            while (elapsedTime < z_transitionDurationToDeath)
            {
                elapsedTime += Time.deltaTime;
                float alpha = Utilities.Lerp(0.0f, 1.0f, elapsedTime / z_transitionDurationToDeath); // 0 -> 1
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
using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Bootstrap : SliceBehaviour
    {
        public static Bootstrap Instance { get; private set; }

        // ============== References ============== 
        private List<IInitializable> initializables = new List<IInitializable>();
        public static CameraController CameraController { get; private set; }
        public static PlayerController Player { get; private set; }
        public static LevelDirector LevelDirector { get; private set; }
        
        public static HUD_Manager HUDManager { get; private set; }

        public bool debug;

        public override void OnAwake()
        {
            if (debug)
            {
                return;
            }

            // Finding references to each script
            GameObject[] arr = gameObject.FindGameObjectsWithTag("Camera");
            if (arr != null) CameraController = arr[0].As<CameraController>();
            //if (CameraController != null) SliceLog.Log("Camera found");
            //else SliceLog.Log("Camera not found");

            arr = gameObject.FindGameObjectsWithTag("Player");
            if (arr != null) Player = arr[0].As<PlayerController>();
            //if (Player != null) SliceLog.Log("Player found");
            //else SliceLog.Log("Player not found");

            arr = gameObject.FindGameObjectsWithTag("Level Director");
            LevelDirector = arr[0].As<LevelDirector>();
            //if (LevelDirector != null) SliceLog.Log("Level Director found");
            //else SliceLog.Log("Level Director not found");


            arr = gameObject.FindGameObjectsWithTag("HUD");
            HUDManager = arr[0].As<HUD_Manager>();
            //if (HUDManager != null) SliceLog.Log("HUD Manager found");
            //else SliceLog.Log("HUD Manager not found");

            // Calling initialize on each script if nothing is empty

            if (CameraController != null && Player != null && HUDManager != null && LevelDirector != null)
            {
                //SliceLog.Log("EVERYTHING IS IN BOOTSTRAP. Initializing");
                CameraController.Initialize();
                Player.Initialize();
                HUDManager.Initialize();
                LevelDirector.Initialize();
            }
            else
            {
                //SliceLog.Log("NOT EVERYTHING IS IN BOOTSTRAP. SPECIFICALLY");
                //if (CameraController == null)   SliceLog.Log("Camera controller");
                //if (Player == null)             SliceLog.Log("Player controller");
                //if (HUDManager == null)         SliceLog.Log("HUD Manager controller");
                //if (LevelDirector == null)      SliceLog.Log("Level Director controller");
            }

            //SliceLog.Log("Boostrap Check finished");
            /*
            if (CameraController != null)   CameraController.Initialize();
            if (Player != null)             Player.Initialize();
            if (HUDManager != null)         HUDManager.Initialize();
            if (LevelDirector != null)      LevelDirector.Initialize();
            */
        }
    }
}

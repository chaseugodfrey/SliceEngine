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
        
        public override void OnCreate()
        {
            base.OnCreate();

            // Finding references to each script
            CameraController = gameObject.FindGameObjectsWithTag("Camera Rig")[0]?.As<CameraController>();
            if (CameraController != null) Console.WriteLine("Camera found");
            else Console.WriteLine("Camera not found");

            Player = gameObject.FindGameObjectsWithTag("Player")[0]?.As<PlayerController>();
            if (Player != null) Console.WriteLine("Player found");
            else Console.WriteLine("Player not found");

            LevelDirector = gameObject.FindGameObjectsWithTag("Level Director")[0]?.As<LevelDirector>();
            if (LevelDirector != null) Console.WriteLine("Level Director found");
            else Console.WriteLine("Level Director not found");


            Console.WriteLine("Awake called");

            // Calling initialize on each script
            CameraController.Initialize();
            Player.Initialize();
            LevelDirector.Initialize();
        }
    }
}

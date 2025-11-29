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
        
        public override void OnAwake()
        {
            // Finding references to each script
            GameObject[] arr = gameObject.FindGameObjectsWithTag("Camera");
            if (arr != null) CameraController = arr[0].As<CameraController>();
            if (CameraController != null) Console.WriteLine("Camera found");
            else Console.WriteLine("Camera not found");

            arr = gameObject.FindGameObjectsWithTag("Player");
            if (arr != null) Player = arr[0].As<PlayerController>();
            if (Player != null) Console.WriteLine("Player found");
            else Console.WriteLine("Player not found");

            arr = gameObject.FindGameObjectsWithTag("Level Director"); 
            LevelDirector = arr[0].As<LevelDirector>();
            if (LevelDirector != null) Console.WriteLine("Level Director found");
            else Console.WriteLine("Level Director not found");

            Console.WriteLine("Jiale called");

            // Calling initialize on each script
            if (CameraController != null) CameraController.Initialize();
            if (Player != null) Player.Initialize();
            if (LevelDirector != null) LevelDirector.Initialize();
        }
    }
}

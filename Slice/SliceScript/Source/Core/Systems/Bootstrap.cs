using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceScript
{
    public class Bootstrap : SliceBehaviour
    {
        public static Bootstrap Instance { get; private set; }

        // ============== References ============== 
        private List<IInitializable> initializables = new List<IInitializable>();
        public static CameraController CameraController { get; private set; }
        public static PlayerController Player { get; private set; }
        
        public override void OnAwake()
        {
            base.OnAwake();

            // Finding references to each script
            CameraController = gameObject.FindGameObjectWithName("Camera").As<CameraController>();
            if (CameraController != null) Console.WriteLine("Camera found");
            else Console.WriteLine("Nothing found");

            Player = gameObject.FindGameObjectWithName("Player").As<PlayerController>();
            if (Player != null) Console.WriteLine("Player found");
            else Console.WriteLine("Nothing found");

            Console.WriteLine("Awake called");
            // Calling initialize on each script
            CameraController.Initialize();
            Player.Initialize();
        }
    }
}

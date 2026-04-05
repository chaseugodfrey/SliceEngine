using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestCamera : SliceBehaviour
    {
        Camera cam;
        bool foundCamera = false;

        public override void OnCreate()
        {

            if(gameObject.FindGameObjectWithName("Camera").HasComponent<Camera>())
            {
                cam = gameObject.FindGameObjectWithName("Camera").GetComponent<Camera>();
                foundCamera = true;
            }
        }

        private void SetImactVars(Vector3 col, Vector3 bgCol, float sharpness, float density, float speed, bool jumps)
        {
            cam.SetImpactFrameColor1(col);
            cam.SetImpactFrameColor2(bgCol);
            cam.SetImpactFrameSharpness(sharpness);
            cam.SetImpactFrameDensity(density);
            cam.SetImpactFrameSpeed(speed);
            cam.SetImpactFrameIsSmooth(jumps);
        }

        public override void OnUpdate(float dt)
        {
            //Console.WriteLine("TestEntity Update called with dt: " + dt);
            if(foundCamera)
            {
                if(Input.IsKeyPressed(Keys.KEY_1))
                    cam.SetImpactFrame(true);
                if (Input.IsKeyPressed(Keys.KEY_2))
                    cam.SetImpactFramePosition(GetComponent<Transform>().WorldPosition);
                if (Input.IsKeyPressed(Keys.KEY_3))
                    SetImactVars(new Vector3(0.0f), new Vector3(1.0f), 100.0f, 20.0f, 1.0f, false);
                if (Input.IsKeyPressed(Keys.KEY_4))
                    SetImactVars(new Vector3(0.0f), new Vector3(1.0f), 0.0f, 50.0f, -0.5f, false);
                if(Input.IsKeyPressed(Keys.KEY_5))
                    SetImactVars(new Vector3(1.0f, 0.0f, 0.0f), new Vector3(0.0f,1.0f,0.0f), 50.0f, 100.0f, -0.2f, false);
                if(Input.IsKeyPressed(Keys.KEY_6))
                    SetImactVars(new Vector3(0.0f), new Vector3(1.0f), 1000.0f, 20.0f, 1.0f, false);
                if(Input.IsKeyPressed(Keys.KEY_7))
                    SetImactVars(new Vector3(0.0f), new Vector3(1.0f), 1000.0f, 2000.0f, 0.2f, true);
                if(Input.IsKeyPressed(Keys.KEY_8))
                    SetImactVars(new Vector3(0.0f), new Vector3(1.0f), 500.0f, 25.0f, 0.2f, true);
                if(Input.IsKeyPressed(Keys.KEY_9))
                    SetImactVars(new Vector3(0.0f), new Vector3(1.0f), 1000.0f, 2000.0f, 1.0f, false);
                if(Input.IsKeyPressed(Keys.KEY_0))
                    cam.SetImpactFrame(false);
                if (Input.IsKeyDown(Keys.KEY_A))
                    cam.FOV += 1.0f;
                if(Input.IsKeyDown(Keys.KEY_D))
                    cam.FOV -= 1.0f;
            }
        }

    }
}

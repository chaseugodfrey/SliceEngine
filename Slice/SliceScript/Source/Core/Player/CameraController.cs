using SliceEngine;
using System;


namespace SliceEngine
{
    public class CameraController : SliceBehaviour, IInitializable
    {
        public float xSensitivity = 100.0f, ySensitivity = 80.0f;

        public override void OnCreate()
        {
            //SliceLog.Log("Rotation: x<" + transform.Rotation.x + ">y<" + transform.Rotation.y + ">z<" + transform.Rotation.z);
        }

        public void Initialize()
        {
        }
        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyDown(Keys.KEY_J)) transform.Rotate(xSensitivity * dt, Vector3.Up);
            else if (Input.IsKeyDown(Keys.KEY_L)) transform.Rotate(-xSensitivity * dt, Vector3.Up);

            if (Input.IsKeyDown(Keys.KEY_I)) transform.Rotate(ySensitivity * dt, Vector3.Right);
            else if (Input.IsKeyDown(Keys.KEY_K)) transform.Rotate(-ySensitivity * dt, Vector3.Right);

            //if (Input.IsKeyPressed(Keys.KEY_Z) || Input.IsKeyDown(Keys.KEY_Z)) transform.Rotate(xSensitivity * dt, new Vector3(1, 0, 0));
            //if (Input.IsKeyPressed(Keys.KEY_X) || Input.IsKeyDown(Keys.KEY_X)) transform.Rotate(ySensitivity * dt, new Vector3(0, 1, 0));
        }
    }
}
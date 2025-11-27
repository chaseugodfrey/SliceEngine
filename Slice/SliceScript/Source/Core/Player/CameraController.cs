using SliceEngine;
using System;


namespace SliceEngine
{
    public class CameraController : SliceBehaviour, IInitializable
    {
        public float xSensitivity = 100.0f, ySensitivity = 80.0f;
        private float pitch = 0f;

        public override void OnCreate()
        {
            //SliceLog.Log("Rotation: x<" + transform.Rotation.x + ">y<" + transform.Rotation.y + ">z<" + transform.Rotation.z);
        }

        public void Initialize()
        {
        }
        public override void OnUpdate(float dt)
        {
            Vector2 mousePos = Input.GetMousePosition();
            //Console.WriteLine("Mouse Position: X=" + mousePos.x + " Y=" + mousePos.y);
            if (Input.IsKeyDown(Keys.KEY_J)) transform.Rotate(xSensitivity * dt, Vector3.Up, true);
            else if (Input.IsKeyDown(Keys.KEY_L)) transform.Rotate(-xSensitivity * dt, Vector3.Up, true);

            // Vertical rotation (pitch) around local X
            float pitchDelta = 0f;
            if (Input.IsKeyDown(Keys.KEY_I)) pitchDelta = ySensitivity * dt;
            else if (Input.IsKeyDown(Keys.KEY_K)) pitchDelta = -ySensitivity * dt;

            float newPitch = Utilities.Clamp(pitch + pitchDelta, -60f, 90f);
            float deltaToApply = newPitch - pitch;
            pitch = newPitch;

            transform.Rotate(deltaToApply, Vector3.Forward);
        }
    }
}
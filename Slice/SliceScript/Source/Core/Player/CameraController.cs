using SliceEngine;
using System;


namespace SliceEngine
{
    public class CameraController : SliceBehaviour, IInitializable
    {
        public Vector3 sensitivity = new Vector3(10f, 8f, 0f);
        public Vector3 yClamp = new Vector3(-40f, 60f, 0f);
        public float resolutionX = 1920, resolutionY = 1080;
        private float pitch = 0f;
        private Vector2 lastMousePos;

        public override void OnCreate()
        {
            //SliceLog.Log("Rotation: x<" + transform.Rotation.x + ">y<" + transform.Rotation.y + ">z<" + transform.Rotation.z);
        }

        public void Initialize()
        {
            //Cursor.state = Cursor.STATE.DISABLED;
        }
        public override void OnUpdate(float dt)
        {
            Vector2 mousePos = Input.GetMousePosition();

            Vector2 delta = mousePos - lastMousePos;
            lastMousePos = mousePos;

            float yawDelta = -delta.x * sensitivity.x * dt;
            float pitchDelta = -delta.y * sensitivity.y * dt;
            transform.Rotate(yawDelta, Vector3.Up, true);

            float newPitch = Utilities.Clamp(pitch + pitchDelta, yClamp.x, yClamp.y);
            float deltaToApply = newPitch - pitch;
            pitch = newPitch;
            transform.Rotate(deltaToApply, Vector3.Right);

            transform.Position = Bootstrap.Player.transform.Position;

            /*
            Vector2 mouseDelta = Input.GetMouseDelta();
            SliceLog.Console("Mouse Delta: X=" + mouseDelta.x + " Y=" + mouseDelta.y);
            float yawDelta = Utilities.Clamp(mouseDelta.x, -90, 90) * sensitivity.x * dt;
            float pitchDelta = Utilities.Clamp(mouseDelta.y, -90, 90) * sensitivity.y * dt;
            transform.Rotate(yawDelta, Vector3.Up, true);

            float clampedPitch = Utilities.Clamp(pitch + pitchDelta, yClamp.x, yClamp.y);
            */
            //float deltaToApply = newPitch - pitch;
            //pitch = newPitch;

            //transform.Rotate(clampedPitch, Vector3.Right);

            //Vector2 mousePos = Input.GetMousePosition();
            //Console.WriteLine("Mouse Position: X=" + mousePos.x + " Y=" + mousePos.y);
            //if (Input.IsKeyDown(Keys.KEY_J)) transform.Rotate(xSensitivity * dt, Vector3.Up, true);
            //else if (Input.IsKeyDown(Keys.KEY_L)) transform.Rotate(-xSensitivity * dt, Vector3.Up, true);

            //// Vertical rotation (pitch) around local X
            //float pitchDelta = 0f;
            //if (Input.IsKeyDown(Keys.KEY_I)) pitchDelta = ySensitivity * dt;
            //else if (Input.IsKeyDown(Keys.KEY_K)) pitchDelta = -ySensitivity * dt;

            //float newPitch = Utilities.Clamp(pitch + pitchDelta, -60f, 90f);
            //float deltaToApply = newPitch - pitch;
            //pitch = newPitch;

            //transform.Rotate(deltaToApply, Vector3.Forward);
        }
    }
}
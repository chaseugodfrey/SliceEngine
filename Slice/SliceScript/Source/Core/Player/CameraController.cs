using SliceEngine;
using System;


namespace SliceEngine
{
    public class CameraController : SliceBehaviour, IInitializable
    {
        public float xSensitivity = 180f, ySensitivity = 130f;
        public void Initialize()
        {

        }
        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyDown(Keys.KEY_J)) transform.Rotate(xSensitivity * dt, transform.Up);
            else if (Input.IsKeyDown(Keys.KEY_L)) transform.Rotate(-xSensitivity * dt, transform.Up);

            if (Input.IsKeyDown(Keys.KEY_I)) transform.Rotate(ySensitivity * dt, transform.Right);
            else if (Input.IsKeyDown(Keys.KEY_K)) transform.Rotate(-ySensitivity * dt, transform.Right);
        }
    }
}
using SliceEngine;
using System;


namespace SliceEngine
{

    public class TestTimeScale : SliceBehaviour
    {
        public float timeScale = 1.0f;
        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_1))
            {
                SliceLog.Console("Pausing");
                timeScale = timeScale == 1.0f ? 0.0f : 1.0f;
                Time.timeScale = timeScale;
            }
        }
    }
}

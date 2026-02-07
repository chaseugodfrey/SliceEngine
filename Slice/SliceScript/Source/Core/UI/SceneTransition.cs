using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SliceEngine;

namespace SliceEngine
{
    public class SceneTransition : SliceBehaviour
    {
        GameObject rect;
        float duration = 2.0f;
        float elapsedTime = 0.0f;
        float t = 0.0f;

        public override void OnCreate()
        {
            rect = FindGameObjectWithName("Transition");
        }

        public override void OnUpdate(float dt)
        {
            SpriteRenderer image = rect.GetComponent<SpriteRenderer>();

            elapsedTime += dt;

            t = Clamp<float>(t, 0, elapsedTime); // Normalize t to 0-1
            float result = a + (b - a) * t; // Becomes 0 + (1-0) * t

        }
    }
}

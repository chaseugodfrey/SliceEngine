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
            SpriteRenderer sr = GetComponent<SpriteRenderer>();

            if (sr != null)
            {
                SceneManager.RegisterTransition(this, sr);
            }
        }

        public override void OnUpdate(float dt)
        {
            
        }
    }
}

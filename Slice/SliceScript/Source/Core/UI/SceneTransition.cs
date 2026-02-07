using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SliceEngine;

namespace SliceEngine
{
    public class SceneTransition : SliceBehaviour
    {
        GameObject rect;

        public override void OnCreate()
        {
            rect = FindGameObjectWithName("Transition");
        }

        public override void OnUpdate(float dt)
        {
            SpriteRenderer image = rect.GetComponent<SpriteRenderer>();
            float duration = 2.0f;
        }
    }
}

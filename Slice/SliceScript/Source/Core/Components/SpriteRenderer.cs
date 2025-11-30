using System;

namespace SliceEngine
{
    public class SpriteRenderer : Component
    {
        public SpriteRenderer() { }
        public SpriteRenderer(GameObject entity)
        {
            gameObject = entity;
        }

        public void SetEnabled(bool enabled)
        {
            FunctionCalls.SpriteRenderer_SetEnabled(gameObject.mID, enabled);
        }
    }
}
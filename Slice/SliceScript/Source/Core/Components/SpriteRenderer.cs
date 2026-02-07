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

        public Vector4 Colour
        {
            get
            {
                FunctionCalls.SpriteRenderer_GetColor(gameObject.mID, out Vector4 value);
                return value;
            }
            set
            {
                FunctionCalls.SpriteRenderer_SetColor(gameObject.mID, ref value);
            }
        }
        public void SetEnabled(bool enabled)
        {
            FunctionCalls.SpriteRenderer_SetEnabled(gameObject.mID, enabled);
        }
    }
}
using System;

namespace SliceEngine
{
    public class Renderer : Component
    {
        public Renderer() { }
        public Renderer(GameObject entity)
        {
            gameObject = entity;
        }

        public void SetColor(Vector4 color)
        {
            FunctionCalls.Material_SetColor(gameObject.mID, ref color);
        }

        public Vector4 GetColor()
        {
            Vector4 color;
            FunctionCalls.Material_GetColor(gameObject.mID, out color);
            return color;
        }

    }
}
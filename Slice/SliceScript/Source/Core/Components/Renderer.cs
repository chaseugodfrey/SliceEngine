using System;

namespace SliceEngine
{
    public class Renderer : Component
    {
        public Renderer() { }
        public Renderer(GameObject entity)
        {
            gameObject = entity;
            ComponentEnabled = FunctionCalls.Renderer_IsEnabled(gameObject.mID);
        }

        public void SetCastShadow(bool castShadow)
        {
            FunctionCalls.Renderer_SetCastShadow(gameObject.mID, castShadow);
        }
        public bool GetCastShadow()
        {
            bool castShadows;
            FunctionCalls.Renderer_GetCastShadow(gameObject.mID, out castShadows);
            return castShadows;
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

        public void SetEmissionColor(Vector4 color)
        {
            FunctionCalls.Material_SetColorEmission(gameObject.mID, ref color);
        }

        public Vector4 GetEmissionColor()
        {
            Vector4 color;
            FunctionCalls.Material_GetColorEmission(gameObject.mID, out color);
            return color;
        }

        public bool ComponentEnabled
        {
            get
            {
                return FunctionCalls.Renderer_IsEnabled(gameObject.mID);
            }
            set
            {
                FunctionCalls.Renderer_SetEnabled(gameObject.mID, value);
            }
        }

    }
}
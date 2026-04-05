using System;

namespace SliceEngine
{
    public class SpriteRendererGammaOverride : Component
    {
        public SpriteRendererGammaOverride() { }
        public SpriteRendererGammaOverride(GameObject entity)
        {
            gameObject = entity;
        }

        public float Gamma
        {
            get
            {
                return FunctionCalls.SpriteGammaOverride_GetGamma(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteGammaOverride_SetGamma(gameObject.mID, value);
            }
        }
        public void SetEnabled(bool enabled)
        {
            FunctionCalls.SpriteGammaOverride_SetEnabled(gameObject.mID, enabled);
        }
    }
}
using System;

namespace SliceEngine
{
    public class SpriteRendererGammaOverrde : Component
    {
        public SpriteRendererGammaOverrde() { }
        public SpriteRendererGammaOverrde(GameObject entity)
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
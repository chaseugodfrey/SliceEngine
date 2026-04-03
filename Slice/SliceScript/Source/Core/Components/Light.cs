using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class Light : Component
    {
        public Light() { }
        public Light(GameObject entity)
        {
            gameObject = entity;
        }

        override public bool enabled
        {
            get { return FunctionCalls.Light_GetEnabled(gameObject.mID); }
            set { FunctionCalls.Light_SetEnabled(gameObject.mID, value); }
        }

        public enum LightType : uint
        {
            DIRECTIONAL,
            POINT,
            SPOT
        }

        public bool CastsShadow
        {
            get { return FunctionCalls.Light_GetCastShadow(gameObject.mID); }
            set { FunctionCalls.Light_SetCastShadow(gameObject.mID, value); }
        }

        public Vector3 Color
        {
            get
            {
                FunctionCalls.Light_GetColor(gameObject.mID, out Vector3 value);
                return value;
            }
            set { FunctionCalls.Light_SetColor(gameObject.mID, ref value); }
        }

        public float Intensity
        {
            get{ return FunctionCalls.Light_GetIntensity(gameObject.mID); }
            set{ FunctionCalls.Light_SetIntensity(gameObject.mID, value); }
        }
        public float Angle
        {
            get{ return FunctionCalls.Light_GetAngle(gameObject.mID); }
            set{ FunctionCalls.Light_SetAngle(gameObject.mID, value); }
        }
        public LightType Type
        {
            get{ return (LightType)FunctionCalls.Light_GetLightType(gameObject.mID); }
            set{ FunctionCalls.Light_SetLightType(gameObject.mID, (int)value); }
        }
    }
}
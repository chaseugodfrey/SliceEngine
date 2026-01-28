using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class ParticleSystem : Component
    {
        public enum ValueType : uint
        {
            CONSTANT,
            CURVE,
            TWO_CONSTANTS
        }

        public Transform parentTransform = null;

        // System Settings
        public float Duration
        {
            get
            {
                FunctionCalls.ParticleSystem_GetDuration(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetDuration(gameObject.mID, ref value);
            }
        }

        public bool IsRepeating
        {
            get
            {
                FunctionCalls.ParticleSystem_GetRepeating(gameObject.mID, out bool value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetRepeating(gameObject.mID, ref value);
            }
        }

        public bool IsLocalSpace
        {
            get
            {
                FunctionCalls.ParticleSystem_GetLocalSpace(gameObject.mID, out bool value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetLocalSpace(gameObject.mID, ref value);
            }
        }

        public bool DestroyOnExpire
        {
            get
            {
                FunctionCalls.ParticleSystem_GetDestroyOnExpire(gameObject.mID, out bool value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetDestroyOnExpire(gameObject.mID, ref value);
            }
        }

        public ulong MaxParticles
        {
            get
            {
                FunctionCalls.ParticleSystem_GetMaxParticles(gameObject.mID, out ulong value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetMaxParticles(gameObject.mID, ref value);
            }
        }

        public float GForce
        {
            get
            {
                FunctionCalls.ParticleSystem_GetGForce(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetGForce(gameObject.mID, ref value);
            }
        }

        public float EmissionRate
        {
            get
            {
                FunctionCalls.ParticleSystem_GetEmissionRate(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetEmissionRate(gameObject.mID, ref value);
            }
        }

        // Shape
        public enum ShapeType
        {
            CONE,
            SPHERE,
            BOX,
            EDGE,
            CIRCLE,
            RECTANGLE
        }

        public ShapeType Shape
        {
            get
            {
                FunctionCalls.ParticleSystem_GetShapeType(gameObject.mID, out ShapeType value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetShapeType(gameObject.mID, ref value);
            }
        }

        public float ConeArc
        {
            get
            {
                FunctionCalls.ParticleSystem_GetConeArc(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetConeArc(gameObject.mID, ref value);
            }
        }

        public float ConeRadius
        {
            get
            {
                FunctionCalls.ParticleSystem_GetConeRadius(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetConeRadius(gameObject.mID, ref value);
            }
        }

        public float SphereRadius
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSphereRadius(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSphereRadius(gameObject.mID, ref value);
            }
        }


        // Scale
        public ValueType ScaleValueType
        {
            get
            {
                FunctionCalls.ParticleSystem_GetScaleValueType(gameObject.mID, out ValueType value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetScaleValueType(gameObject.mID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                FunctionCalls.ParticleSystem_GetScale(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetScale(gameObject.mID, ref value);
            }
        }

        public Vector3 ScaleMin
        {
            get
            {
                FunctionCalls.ParticleSystem_GetScaleMin(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetScaleMin(gameObject.mID, ref value);
            }
        }

        public Vector3 ScaleMax
        {
            get
            {
                FunctionCalls.ParticleSystem_GetScaleMax(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetScaleMax(gameObject.mID, ref value);
            }
        }

        // Lifetime
        public ValueType InitialLifetimeType
        {
            get
            {
                FunctionCalls.ParticleSystem_GetLifetimeValueType(gameObject.mID, out ValueType value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetLifetimeValueType(gameObject.mID, ref value);
            }
        }

        public float Lifetime
        {
            get
            {
                FunctionCalls.ParticleSystem_GetLifetime(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetLifetime(gameObject.mID, ref value);
            }
        }

        public float ParticleLifetimeMin
        {
            get
            {
                FunctionCalls.ParticleSystem_GetMinLifetime(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetMinLifetime(gameObject.mID, ref value);
            }
        }

        public float ParticleLifetimeMax
        {
            get
            {
                FunctionCalls.ParticleSystem_GetMaxLifetime(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetMaxLifetime(gameObject.mID, ref value);
            }
        }

        // Rotation
        public ValueType RotationType
        {
            get
            {
                FunctionCalls.ParticleSystem_GetRotationValueType(gameObject.mID, out ValueType value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetRotationValueType(gameObject.mID, ref value);
            }
        }

        public float Rotation
        {
            get
            {
                FunctionCalls.ParticleSystem_GetRotation(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetRotation(gameObject.mID, ref value);
            }
        }

        public float RotationMin
        {
            get
            {
                FunctionCalls.ParticleSystem_GetRotationMin(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetRotationMin(gameObject.mID, ref value);
            }
        }

        public float RotationMax
        {
            get
            {
                FunctionCalls.ParticleSystem_GetRotationMax(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetRotationMax(gameObject.mID, ref value);
            }
        }

        public bool IsRotation3D
        {
            get
            {
                FunctionCalls.ParticleSystem_GetIsRotation3D(gameObject.mID, out bool value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetIsRotation3D(gameObject.mID, ref value);
            }
        }


        public Vector3 Rotation3D
        {
            get
            {
                FunctionCalls.ParticleSystem_GetRotation3DHint(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetRotation3DHint(gameObject.mID, ref value);
            }
        }

        public Vector3 MinRotation3D
        {
            get
            {
                FunctionCalls.ParticleSystem_GetMinRotation3DHint(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetMinRotation3DHint(gameObject.mID, ref value);
            }
        }

        public Vector3 MaxRotation3D
        {
            get
            {
                FunctionCalls.ParticleSystem_GetMaxRotation3DHint(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetMaxRotation3DHint(gameObject.mID, ref value);
            }
        }


        // Spawn Position
        public ValueType SpawnPosValueType
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpawnPosValueType(gameObject.mID, out ValueType value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpawnPosValueType(gameObject.mID, ref value);
            }
        }

        public Vector3 SpawnPos
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpawnPos(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpawnPos(gameObject.mID, ref value);
            }
        }

        public Vector3 SpawnPosMin
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpawnPosMin(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpawnPosMin(gameObject.mID, ref value);
            }
        }

        public Vector3 SpawnPosMax
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpawnPosMax(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpawnPosMax(gameObject.mID, ref value);
            }
        }

        // Colour
        public ValueType ColourValueType
        {
            get
            {
                FunctionCalls.ParticleSystem_GetColourValueType(gameObject.mID, out ValueType value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetColourValueType(gameObject.mID, ref value);
            }
        }

        public Vector4 Colour
        {
            get
            {
                FunctionCalls.ParticleSystem_GetColour(gameObject.mID, out Vector4 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetColour(gameObject.mID, ref value);
            }
        }

        public Vector4 ColourMin
        {
            get
            {
                FunctionCalls.ParticleSystem_GetColourMin(gameObject.mID, out Vector4 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetColourMin(gameObject.mID, ref value);
            }
        }

        public Vector4 ColourMax
        {
            get
            {
                FunctionCalls.ParticleSystem_GetColourMax(gameObject.mID, out Vector4 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetColourMax(gameObject.mID, ref value);
            }
        }


        // Speed
        public ValueType SpeedValueType
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpeedValueType(gameObject.mID, out ValueType value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpeedValueType(gameObject.mID, ref value);
            }
        }
        public float Speed
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpeed(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpeed(gameObject.mID, ref value);
            }
        }
        public float SpeedMin
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpeedMin(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpeedMin(gameObject.mID, ref value);
            }
        }
        public float SpeedMax
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSpeedMax(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSpeedMax(gameObject.mID, ref value);
            }
        }

        // Col over lifetime
        public bool ColourOverLifetime
        {
            get
            {
                FunctionCalls.ParticleSystem_GetColourOverLifetime(gameObject.mID, out bool value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetColourOverLifetime(gameObject.mID, ref value);
            }
        }

        // Collision
        public bool HasCollision
        {
            get
            {
                FunctionCalls.ParticleSystem_GetCollision(gameObject.mID, out bool value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetCollision(gameObject.mID, ref value);
            }
        }

        // Renderer / Resources
        public uint TextureID
        {
            get
            {
                FunctionCalls.ParticleSystem_GetTextureID(gameObject.mID, out uint value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetTextureID(gameObject.mID, ref value);
            }
        }


        public bool Expired
        {
            get
            {
                FunctionCalls.ParticleSystem_IsExpired(gameObject.mID, out bool value);
                return value;
            }
        }

        public float SystemTimer
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSystemTimer(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetSystemTimer(gameObject.mID, ref value);
            }
        }

        public ParticleSystem() {}

        public ParticleSystem(GameObject entity)
        {
            gameObject = entity;
        }
    }
}

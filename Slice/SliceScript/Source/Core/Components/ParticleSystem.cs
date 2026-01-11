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

        private float _duration;
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
        // 0 = forever

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


        // Lifetime
        public ValueType initialLifetimeType = ValueType.CONSTANT;

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

        public float minParticleLifetime
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

        public float maxParticleLifetime
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
        public bool isInitialRotation3D = false;
        public ValueType initialRotationType = ValueType.CONSTANT;
        public Quaternion rotation = Quaternion.Identity;
        public Quaternion minRandomRotation = Quaternion.Identity;
        public Quaternion maxRandomRotation = Quaternion.Identity;
        public Vector3 eulerHint = Vector3.Zero;
        public Vector3 minEulerHint = Vector3.Zero;
        public Vector3 maxEulerHint = Vector3.Zero;

        public void Set1DRotation(float val)
        {
            eulerHint.x = val;
        }

        public float Get1DRotation()
        {
            return eulerHint.x;
        }

        // Size / Scale
        public ValueType scaleType = ValueType.CONSTANT;
        public Vector3 scale = Vector3.One;
        public Vector3 minRandomScale = Vector3.One;
        public Vector3 maxRandomScale = Vector3.One;

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

        // Emission
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

        public class Burst
        {
            public ulong numParticles = 0;
            public ulong burstRepetitions = 0;
            public float burstPeriod = 0f;
            public float triggerTime = 0f;
            public bool triggered = false;

            public ulong repsDone = 0;
            public float repTimer = 0f;
        }

        public List<Burst> bursts = new List<Burst>();

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

        public float ConeAngle
        {
            get
            {
                FunctionCalls.ParticleSystem_GetConeAngle(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetConeAngle(gameObject.mID, ref value);
            }
        }

        public float ShapeRadius
        {
            get
            {
                FunctionCalls.ParticleSystem_GetShapeRadius(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetShapeRadius(gameObject.mID, ref value);
            }
        }

        public float ShapeArc
        {
            get
            {
                FunctionCalls.ParticleSystem_GetShapeArc(gameObject.mID, out float value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetShapeArc(gameObject.mID, ref value);
            }
        }

        public Vector3 axis = Vector3.Zero;

        // Initial Position Offset
        public ValueType spawnPosValueType= ValueType.CONSTANT;
        public Vector3 spawnPositionOffset
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

        public Vector3 minRandomSpawnPos = Vector3.Zero;
        public Vector3 maxRandomSpawnPos = Vector3.Zero;

        // Color
        public ValueType colorValueType = ValueType.CONSTANT;
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

        public bool ColorOverLifetime
        {
            get
            {
                FunctionCalls.ParticleSystem_GetColorOverLifetime(gameObject.mID, out bool value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetColorOverLifetime(gameObject.mID, ref value);
            }
        }

        public Vector4 minRandomColour = new Vector4(0, 0, 0, 1);
        public Vector4 maxRandomColour = new Vector4(0, 0, 0, 1);

        public Dictionary<float, Vector4> colorLifeTimeMap = new Dictionary<float, Vector4>();

        // Velocity
        public ValueType velocityValueType = ValueType.CONSTANT;
        public Vector3 Velocity
        {
            get
            {
                FunctionCalls.ParticleSystem_GetVelocity(gameObject.mID, out Vector3 value);
                return value;
            }
            set
            {
                FunctionCalls.ParticleSystem_SetVelocity(gameObject.mID, ref value);
            }
        }
        public Vector3 minRandomVelocity = Vector3.One;
        public Vector3 maxRandomVelocity = Vector3.One;

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

        public enum RenderMode { BILLBOARD, MESH }
        public RenderMode renderMode = RenderMode.BILLBOARD;

        public bool systemEnding = false;
        public bool Expired
        {
            get
            {
                FunctionCalls.ParticleSystem_IsExpired(gameObject.mID, out bool value);
                return value;
            }
        }
        public bool isActive = true;

        public float SystemTimer
        {
            get
            {
                FunctionCalls.ParticleSystem_GetSystemTimer(gameObject.mID, out float value);
                return value;
            }
        }

        public ParticleSystem() {}

        public ParticleSystem(GameObject entity)
        {
            gameObject = entity;
        }
    }
}

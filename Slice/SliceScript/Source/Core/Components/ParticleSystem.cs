using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class ParticleSystem : Component
    {
        public enum ValueType
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

        public bool isRepeating;
        {
            get
            {
                Functionalls
            }
        }
        public bool isLocalSpace = false;

        // Lifetime
        public ValueType initialLifetimeType = ValueType.CONSTANT;
        public float lifetime = 0f;
        public float minParticleLifetime = 0f;
        public float maxParticleLifetime = 0f;

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

        public bool destroyOnExpire = false;
        public ulong maxParticles = 1000;

        public float gForce = 0f;

        // Emission
        public float emissionRate = 0f;

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

        public ShapeType shapeType = ShapeType.CONE;

        public float coneAngle = 0f;
        public float shapeRadius = 0f;
        public float shapeArc = 0f;

        public Vector3 axis = Vector3.Zero;

        // Initial Position
        public bool hasRandomSpawnPos = false;
        public Vector3 minRandomSpawnPos = Vector3.Zero;
        public Vector3 maxRandomSpawnPos = Vector3.Zero;

        // Color
        public ValueType colorValueType = ValueType.CONSTANT;
        public Vector4 colour = new Vector4(0, 0, 0, 1);
        public Vector4 minRandomColour = new Vector4(0, 0, 0, 1);
        public Vector4 maxRandomColour = new Vector4(0, 0, 0, 1);

        public bool colorOverLifetime = false;
        public Dictionary<float, Vector4> colorLifeTimeMap = new Dictionary<float, Vector4>();

        // Velocity
        public ValueType velocityValueType = ValueType.CONSTANT;
        public Vector3 velocity = Vector3.One;
        public Vector3 minRandomVelocity = Vector3.One;
        public Vector3 maxRandomVelocity = Vector3.One;

        // Collision
        public bool hasCollision = false;

        // Renderer / Resources
        public uint textureID; // or your custom type
        public enum RenderMode { BILLBOARD, MESH }
        public RenderMode renderMode = RenderMode.BILLBOARD;

        public bool systemEnding = false;
        public bool expired = false;
        public bool isActive = true;

        public float systemTimer = 0f;

        public ParticleSystem() {}

        public ParticleSystem(GameObject entity)
        {
            gameObject = entity;
        }
    }
}

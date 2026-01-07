using System;

namespace SliceEngine
{
    public enum QueryTriggerInteraction
    {
        UseGlobal,    // Use the global Physics.queriesHitTriggers setting
        Ignore,       // Never hit triggers
        Collide       // Always hit triggers
    }

    public struct Physics
    {
        public static bool queriesHitTriggers = true;

        public const uint DefaultRaycastLayers = ~0u; // All layers
        public bool RayCast(Vector3 origin, Vector3 direction, float maxDistance = float.PositiveInfinity, uint layerCast = DefaultRaycastLayers, QueryTriggerInteraction queryTriggerInteraction = QueryTriggerInteraction.UseGlobal)
        {
            return true;
        }

        public bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hitInfo, float maxDistance, int layerMask, QueryTriggerInteraction queryTriggerInteraction)
        {

        }

        public bool SphereCast()
        {
            return true;
        }
    }
}
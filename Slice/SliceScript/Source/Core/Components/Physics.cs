using System;
using System.IO;

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

        //public static QueryTriggerInteraction globalInteraction = QueryTriggerInteraction.Collide;

        public static void RayUpdateMovement(uint entityID, Vector3 d_m)
        {
            FunctionCalls.Physics_RayUpdateMovement(entityID, out d_m);
        }
        public static bool RayCast(Vector3 origin, Vector3 direction,uint layerMask = DefaultRaycastLayers, QueryTriggerInteraction queryTriggerInteraction = QueryTriggerInteraction.UseGlobal)
        {
            //uint bodyHitID = 0;


            //FunctionCalls.Physics_Raycast(out origin, out direction, ref bodyHitID, layerMask);


            return true;
        }

        public static bool Raycast(Vector3 origin, Vector3 direction, out RayCastHit hitInfo, uint layerMask, QueryTriggerInteraction queryTriggerInteraction)
        {
            hitInfo = new RayCastHit();
            bool triggerInteraction = false;
            uint bodyHitID = 0;

            if (queryTriggerInteraction == QueryTriggerInteraction.UseGlobal)
            {
                if (queriesHitTriggers)
                {
                    triggerInteraction = true;
                }
                else
                {
                    triggerInteraction = false;
                }
            }
            else if (queryTriggerInteraction == QueryTriggerInteraction.Ignore)
            {
                triggerInteraction = false;
            }
            else if (queryTriggerInteraction == QueryTriggerInteraction.Collide)
            {
                triggerInteraction = true;
            }  
            bool test = FunctionCalls.Physics_Raycast(out origin, out direction, ref bodyHitID, ref hitInfo.point, ref hitInfo.normal, triggerInteraction, layerMask);
            hitInfo.distance = (hitInfo.point - origin).Magnitude();
            GameObject obj = new GameObject(bodyHitID);
            hitInfo.transform =  obj.GetComponent<Transform>();

            return test;
        }

        public static bool Raycast(Ray ray, uint layerMask = DefaultRaycastLayers, QueryTriggerInteraction queryTriggerInteraction = QueryTriggerInteraction.UseGlobal)
        {
            return true;
        }

        public static bool Raycast(Ray ray, out RayCastHit hitInfo, uint layerMask = DefaultRaycastLayers, QueryTriggerInteraction queryTriggerInteraction = QueryTriggerInteraction.UseGlobal)
        {
            hitInfo = new RayCastHit();
            return true;
        }

        public static void DebugDrawRay(Vector3 origin, Vector3 direction, float magnitute)
        {
            FunctionCalls.Physics_DrawRay(ref origin, ref direction, magnitute);
        }

        public bool SphereCast()
        {
            return true;
        }
    }
}
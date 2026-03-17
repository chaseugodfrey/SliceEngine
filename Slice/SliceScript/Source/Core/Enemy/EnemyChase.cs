using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyChase : SliceBehaviour
    {
        public float moveSpeed = 5.0f;
        public string targetObject;

        GameObject targetObjRef;

        GameObject playerObjRef;

        NavAgent navAgent;

        float pathUpdateTimer = 0.0f;
        float pathUpdateInterval = 0.2f;

        public override void OnCreate()
        {
            FunctionCalls.Log("EnemyChase: OnCreate");
            navAgent = GetComponent<NavAgent>();

            if (navAgent != null)
            {
                navAgent.Speed = moveSpeed;
            }
            else
            {
                FunctionCalls.LogWarn("EnemyChase: Missing NavAgent component!");
            }

            if (!string.IsNullOrEmpty(targetObject))
            {
                targetObjRef = gameObject.FindGameObjectWithName(targetObject);
                if (targetObjRef == null)
                {
                    FunctionCalls.LogWarn($"EnemyChase: Target '{targetObject}' not found. Falling back to RootNode.");
                }
            }

            if (targetObjRef == null)
            {
                playerObjRef = gameObject.FindGameObjectWithName("RootNode");
            }
        }

        public override void OnUpdate(float dt)
        {
            if (navAgent != null)
            {
                GameObject activeTarget = targetObjRef != null ? targetObjRef : playerObjRef;

                if (activeTarget != null)
                {
                    pathUpdateTimer += dt;

                    if (pathUpdateTimer > pathUpdateInterval)
                    {
                        pathUpdateTimer = 0.0f;

                        Vector3 targetPos = activeTarget.GetComponent<Transform>().Position;

                        navAgent.SetDestination(targetPos);

                        // Debug print
                        // FunctionCalls.Log($"Chasing... Target Pos: {targetPos.x}, {targetPos.z}");
                    }
                }
            }
        }

        public override void OnCollideEnter(uint other) { }
        public override void OnCollideStay(uint other) { }
    }
}
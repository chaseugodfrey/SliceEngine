/*using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyChase : SliceBehaviour
    {
        public float moveSpeed = 5.0f;
        public string targetObject; // Make sure this is set in the editor!

        Transform targetTransform;
        Transform enemyT;
        Transform playerT = null;
        public Vector3[] test;

        public override void OnCreate()
        {
            FunctionCalls.Log("EnemyChase: OnCreate called! Script is active.");
            // 1. Get our own Transform
            enemyT = GetComponent<Transform>();

            // 2. Safely find the Player ("RootNode")
            // We cannot chain .GetComponent() immediately because Find might return null.
            GameObject playerObj = gameObject.FindGameObjectWithName("RootNode");

            if (playerObj != null)
            {
                playerT = playerObj.GetComponent<Transform>();
            }
            else
            {
                // Log a warning so you know why the enemy isn't moving
                FunctionCalls.LogWarn("EnemyChase: Could not find GameObject named 'RootNode'!");
            }

            // 3. Safely find the dynamic Target (if one is set)
            if (!string.IsNullOrEmpty(targetObject))
            {
                GameObject targetObj = gameObject.FindGameObjectWithName(targetObject);
                if (targetObj != null)
                {
                    targetTransform = targetObj.GetComponent<Transform>();
                }
                else
                {
                    FunctionCalls.LogWarn($"EnemyChase: Could not find target object named '{targetObject}'");
                }
            }
        }

        public override void OnUpdate(float dt)
        {
            // Move towards dynamic target if it exists
            if (targetTransform != null)
            {
                if (HasComponent<NavAgent>())
                {
                    if (Input.IsKeyPressed(Keys.KEY_SPACEBAR))
                    {
                        FunctionCalls.Log("EnemyChase: Moving towards target.");
                        NavAgent agent = GetComponent<NavAgent>();
                        agent.SetDestination(targetTransform.Position);
                    }
                }
            }
            // Fallback: Use RootNode/Player if dynamic target is null
            else if (playerT != null)
            {
                // (Optional) Your old movement logic or NavAgent logic here
                Vector3 direction_diff = playerT.Position - enemyT.Position;
                enemyT.Position += direction_diff.Normalize() * moveSpeed * dt;
            }
        }

        public override void OnCollideEnter(uint other) { }
        public override void OnCollideStay(uint other) { }
    }
}*/

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

        Transform targetTransform;
        Transform enemyT;
        NavAgent navAgent; // Cache this!

        // Timer to update path (don't spam C++ every frame)
        float pathUpdateTimer = 0.0f;
        float pathUpdateInterval = 0.2f; // Update path 5 times a second

        public override void OnCreate()
        {
            FunctionCalls.Log("EnemyChase: OnCreate");
            enemyT = GetComponent<Transform>();
            navAgent = GetComponent<NavAgent>();

            // Ensure speed is passed to the NavAgent
            if (navAgent != null)
            {
                navAgent.Speed = moveSpeed;
            }
            else
            {
                FunctionCalls.LogWarn("EnemyChase: Missing NavAgent component!");
            }

            // Find Target
            if (!string.IsNullOrEmpty(targetObject))
            {
                GameObject targetObj = gameObject.FindGameObjectWithName(targetObject);
                if (targetObj != null)
                {
                    targetTransform = targetObj.GetComponent<Transform>();
                }
                else
                {
                    FunctionCalls.LogWarn($"EnemyChase: Target '{targetObject}' not found. Falling back to RootNode.");
                }
            }

            // Fallback to Player/RootNode if targetObject was empty or not found
            if (targetTransform == null)
            {
                GameObject playerObj = gameObject.FindGameObjectWithName("RootNode");
                if (playerObj != null) targetTransform = playerObj.GetComponent<Transform>();
            }
        }

        public override void OnUpdate(float dt)
        {
            if (navAgent != null && targetTransform != null)
            {
                pathUpdateTimer += dt;

                // Periodically update the destination to follow the target
                if (pathUpdateTimer > pathUpdateInterval)
                {
                    pathUpdateTimer = 0.0f;
                    navAgent.SetDestination(targetTransform.Position);

                    // Debug print to confirm it's trying to move
                    FunctionCalls.Log($"Chasing... Target Pos: {targetTransform.Position.x}, {targetTransform.Position.z}");
                }
            }
        }

        public override void OnCollideEnter(uint other) { }
        public override void OnCollideStay(uint other) { }
    }
}
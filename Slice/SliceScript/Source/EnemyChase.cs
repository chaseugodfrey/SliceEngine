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
}
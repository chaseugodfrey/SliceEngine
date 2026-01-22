using SliceEngine;
using System;

namespace SliceEngine
{
    internal class PlayerTestNavAgent : SliceBehaviour
    {
        GameObject floor;
        GameObject enemy;
        NavAgent enemyAgent;
        RigidBody enemyRb;

        bool grounded = true;
        bool isEnemyKnockedBack = false;
        float knockbackTimer = 0.0f;

        public override void OnCreate()
        {
            // Cache references once at startup
            floor = FindGameObjectWithName("Floor");
            if (floor == null) FunctionCalls.LogWarn("PlayerTest: 'Floor' not found!");

            enemy = FindGameObjectWithName("Enemy");
            if (enemy != null)
            {
                enemyAgent = enemy.GetComponent<NavAgent>();
                enemyRb = enemy.GetComponent<RigidBody>();
            }
            else
            {
                FunctionCalls.LogWarn("PlayerTest: 'Enemy' not found!");
            }
        }

        public override void OnUpdate(float dt)
        {
            // Monitor Enemy Landing Logic
            if (isEnemyKnockedBack && enemyRb != null && enemyAgent != null)
            {
                knockbackTimer += dt;

                // Wait a bit before checking for landing (to let it fly up first)
                if (knockbackTimer > 0.5f)
                {
                    // Simple landing check: Is velocity low? Is it close to Y=0 (or whatever floor height is)?
                    // Or relies on the enemy's own collision logic if available.
                    // For now, let's just use a timer + height check as a proxy for "Landed"

                    float enemyY = enemy.GetComponent<Transform>().Position.y;

                    // If enemy is falling/on ground (approx 0.5 height)
                    if (enemyY < 1.0f)
                    {
                        // Re-enable Navigation
                        enemyAgent.SetComponentIsEnabled(enemy, true);
                        isEnemyKnockedBack = false;
                        knockbackTimer = 0.0f;
                        FunctionCalls.Log("Enemy Landed: Resuming Navigation");
                    }
                }
            }
        }

        public override void OnCollideEnter(uint other)
        {
            // Hit Enemy Logic
            if (enemy != null && other == enemy.mID)
            {
                FunctionCalls.Log("PlayerTest: OnCollideEnter");
                // 1. Disable NavAgent so Physics takes full control
                if (enemyAgent != null && enemyAgent.ComponentIsEnabled(enemy))
                {
                    enemyAgent.SetComponentIsEnabled(enemy, false);
                    isEnemyKnockedBack = true;
                    knockbackTimer = 0.0f; // Reset timer
                }

                // 2. Apply Knockback Force
                if (enemyRb != null)
                {
                    // Apply impulse UP and AWAY
                    // Assuming player forward or just generic direction
                    Vector3 force = new Vector3(5.0f, 8.0f, 0.0f);
                    enemyRb.AddForce(force, ForceMode.Impulse);
                    FunctionCalls.Log("Collision with Enemy: Applied Knockback (Nav Disabled)");
                }
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (floor != null && other == floor.mID)
            {
                grounded = false;
            }
        }
    }
}
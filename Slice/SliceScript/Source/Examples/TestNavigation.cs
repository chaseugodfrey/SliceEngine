/*using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestNavigation : SliceBehaviour
    {
        private NavAgent agent;

        public override void OnCreate()
        {
            // 1. Try to get the NavAgent component
            // This tests if the Component wrapper and Entity ID mapping are correct
            agent = gameObject.GetComponent<NavAgent>();

            if (agent != null)
            {
                FunctionCalls.Log("TestNavigation: NavAgent component found successfully.");

                // 2. Test Property Setters
                // Set a visible speed to ensure values are passing to C++
                agent.Speed = 4.5f;
                FunctionCalls.Log($"TestNavigation: Agent speed set to {agent.Speed}");

                // 3. Initial Move Test
                // Send it to a default position immediately on start
                Vector3 startPos = new Vector3(5.0f, 0.0f, 5.0f);
                agent.SetDestination(startPos);
                FunctionCalls.Log("TestNavigation: Initial destination set to (5, 0, 5)");
            }
            else
            {
                FunctionCalls.LogError("TestNavigation: FAILED to find NavAgent component!");
            }
        }

        public override void OnUpdate(float dt)
        {
            // Guard clause in case component was missing
            if (agent == null) return;

            // --- Interactive Tests ---

            // Press 'T' to go to a "Target"
            if (Input.IsKeyPressed(Keys.KEY_T))
            {
                Vector3 target = new Vector3(10.0f, 0.0f, 10.0f);
                agent.SetDestination(target);
                FunctionCalls.Log("TestNavigation: User commanded move to (10, 0, 10)");
            }

            // Press 'O' to go back to "Origin"
            if (Input.IsKeyPressed(Keys.KEY_O))
            {
                Vector3 origin = new Vector3(0.0f, 0.0f, 0.0f);
                agent.SetDestination(origin);
                FunctionCalls.Log("TestNavigation: User commanded move to Origin");
            }

            // Press 'S' to Stop
            if (Input.IsKeyPressed(Keys.KEY_S))
            {
                agent.Stop();
                FunctionCalls.Log("TestNavigation: User commanded Stop");
            }

            // Uncomment this if you want to spam the console to check path status
            *//*
            if (agent.HasPath)
            {
                // FunctionCalls.Log("Agent is moving...");
            }
            *//*
        }
    }
}*/
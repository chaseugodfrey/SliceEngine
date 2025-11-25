using System;
using SliceEngine;

namespace SliceEngine
{
    public class NavAgent : Component
    {
        public NavAgent() { }

        public NavAgent(GameObject entity)
        {
            gameObject = entity;
        }

        public float Speed
        {
            get
            {
                if (gameObject == null) return 0f;
                return FunctionCalls.NavAgent_GetSpeed(gameObject.mID);
            }
            set
            {
                if (gameObject != null)
                {
                    FunctionCalls.NavAgent_SetSpeed(gameObject.mID, value);
                }
            }
        }

        public bool HasPath
        {
            get
            {
                if (gameObject == null) return false;
                return FunctionCalls.NavAgent_HasPath(gameObject.mID);
            }
        }

        public void SetDestination(Vector3 target)
        {
            if (gameObject != null)
            {
                // We pass by ref because your C++ internal call expects a pointer
                FunctionCalls.NavAgent_SetDestination(gameObject.mID, ref target);
            }
        }

        public void Stop()
        {
            if (gameObject != null)
            {
                FunctionCalls.NavAgent_Stop(gameObject.mID);
            }
        }
    }
}
using System;

namespace SliceEngine
{
    public enum ForceMode
    {
        Force,
        Impulse,
        VelocityChange,
        Acceleration
    }

    public class RigidBody : Component
    {
        public Vector3 Velocity
        {
            get
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");
                FunctionCalls.RigidBody_GetVelocity(gameObject.mID, out Vector3 position);
                return position;
            }
            set
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");

                FunctionCalls.RigidBody_SetVelocity(gameObject.mID, ref value);
            }
        }

        public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
        {
            FunctionCalls.RigidBody_AddForce(gameObject.mID, out force, (int)mode);
        }
    }
}
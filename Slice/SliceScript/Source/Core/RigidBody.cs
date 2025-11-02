using System;

namespace SliceEngine
{
    public class RigidBody : Component
    {
        public Vector3 Velocity
        {
            get
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");
                FunctionCalls.RigidBody_GetVelocity(Entity.mID, out Vector3 position);
                return position;
            }
            set
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");

                FunctionCalls.RigidBody_SetVelocity(Entity.mID, ref value);
            }
        }

        public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
        {
            FunctionCalls.RigidBody_AddForce(Entity.mID, out force, (int)mode);
        }
    }
}
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

        public RigidBody(GameObject entity)
        {
            gameObject = entity;
            FunctionCalls.RigidBody_GetVelocity(gameObject.mID, out Vector3 vel);
            gravityFactor = FunctionCalls.RigidBody_GetGravityFactor(gameObject.mID);
            Velocity = vel;
        }

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

        public float gravityFactor
        {
            get
            {
               return FunctionCalls.RigidBody_GetGravityFactor(gameObject.mID);
            }
            set
            {
                FunctionCalls.RigidBody_SetGravityFactor(gameObject.mID, value);
            }
        }

        public bool IsGravityOff()
        {
                return FunctionCalls.RigidBody_IsGravityOff(gameObject.mID);
        }

        public void OffGravity(bool isOff)
        {
              FunctionCalls.RigidBody_OffGravity(gameObject.mID, isOff);
        }

        public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
        {
            FunctionCalls.RigidBody_AddForce(gameObject.mID, out force, (int)mode);
        }
    }
}
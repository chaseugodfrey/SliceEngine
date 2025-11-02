using System;

namespace SliceEngine
{
    public class Transform : Component
    {
        public Vector3 Position
        {
            get
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");
                FunctionCalls.Transform_GetPosition(Entity.mID, out Vector3 position);
                return position;
            }
            set
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");

                FunctionCalls.Transform_SetPosition(Entity.mID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                FunctionCalls.Transform_GetScale(Entity.mID, out Vector3 scale);
                return scale;
            }
            set
            {
                FunctionCalls.Transform_SetScale(Entity.mID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                FunctionCalls.Transform_GetRotation(Entity.mID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                FunctionCalls.Transform_SetRotation(Entity.mID, ref value);
            }
        }

    }
}
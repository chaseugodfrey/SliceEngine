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

        public void Rotate(float angleDegrees, Vector3 axis)
        {
            Vector3 rotation = this.Rotation;

            // Assuming axis is exactly along X, Y, or Z
            if (axis.x != 0) rotation.x += angleDegrees;
            if (axis.y != 0) rotation.y += angleDegrees;
            if (axis.z != 0) rotation.z += angleDegrees;

            // Optional: keep angles between 0-360
            rotation.x = rotation.x % 360f;
            rotation.y = rotation.y % 360f;
            rotation.z = rotation.z % 360f;

            this.Rotation = rotation;
        }

    }
}
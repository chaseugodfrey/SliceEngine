using System;

namespace SliceEngine
{
    public struct Vector3
    {
        public float x, y, z;

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public Vector3(float val)
        {
            this.x = val;
            this.y = val;
            this.z = val;
        }

        public static Vector3 Zero => new Vector3(0.0f);

        public static Vector3 operator -(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
        }

        public static Vector3 operator +(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
        }

        public static Vector3 operator *(Vector3 v1, float f1)
        {
            return new Vector3(v1.x * f1, v1.y * f1, v1.z * f1);
        }

        public static bool operator ==(Vector3 v1, Vector3 v2)
        {
            return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
        }

        public static bool operator !=(Vector3 v1, Vector3 v2)
        {
            return !(v1 == v2);
        }

        public Vector3 Normalize()
        {
            float magnitude = (float)Math.Sqrt(x * x + y * y + z * z);
            if (magnitude > 0)
            {
                return new Vector3(x / magnitude, y / magnitude, z / magnitude);
            }
            else
            {
                return Vector3.Zero;
            }
        }

        public float Distance(Vector3 v)
        {
            float dx = v.x - x;
            float dy = v.y - y;
            float dz = v.z - z;
            return (float)Math.Sqrt(dx * dx + dy * dy + dz * dz);
        }

    }
}
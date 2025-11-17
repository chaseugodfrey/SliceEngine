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

        public override bool Equals(object obj)
        {
            if (obj is Vector3 other)
            {
                return x == other.x && y == other.y && z == other.z;
            }
            return false;
        }
        public static Vector3 Cross(Vector3 a, Vector3 b)
        {
            return new Vector3(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }

        public float LengthSquared()
        {
            return x * x + y * y + z * z;
        }

        public float Length()
        {
            return (float)Math.Sqrt(LengthSquared());
        }

        public override int GetHashCode()
        {
            // A common way to combine hash codes on older frameworks
            unchecked // Overflow is fine, just wrap
            {
                int hash = 17;
                hash = hash * 23 + x.GetHashCode();
                hash = hash * 23 + y.GetHashCode();
                hash = hash * 23 + z.GetHashCode();
                return hash;
            }
        }

        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
            {
                return min;
            }
            if (value > max)
            {
                return max;
            }
            return value;
        }

        public static float Dot(Vector3 v1, Vector3 v2)
        {
            return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
        }

        public static Vector3 Slerp(Vector3 start, Vector3 end, float t)
        {
            t = Vector3.Clamp(t, 0.0f, 1.0f);
            float dot = Vector3.Dot(start, end);
            dot = Vector3.Clamp(dot, -1.0f, 1.0f);
            float omega = (float)Math.Acos(dot);
            if (omega < 0.0001f)
            {
                Vector3 tmp  = (start + (end - start) * t);
                return tmp.Normalize();
            }
            float sinOmega = (float)Math.Sin(omega);
            float scale0 = (float)Math.Sin((1.0f - t) * omega) / sinOmega;
            float scale1 = (float)Math.Sin(t * omega) / sinOmega;
            Vector3 result = (start * scale0) + (end * scale1);

            return result;
        }
    }
}
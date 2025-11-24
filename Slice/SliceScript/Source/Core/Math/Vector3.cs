using System;
using System.IO;

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
        public static Vector3 operator /(Vector3 v, float f)
        {
            if (Math.Abs(f) < Utilities.Epsilon)
                throw new DivideByZeroException("Cannot divide a Vector3 by zero.");

            return new Vector3(v.x / f, v.y / f, v.z / f);
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
        public float Magnitude()
        {
            return (float)Math.Sqrt(x * x + y * y + z * z); 
        }

        public float Distance(Vector3 v)
        {
            float dx = v.x - x;
            float dy = v.y - y;
            float dz = v.z - z;
            return (float)Math.Sqrt(dx * dx + dy * dy + dz * dz);
        }
        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
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

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            // Clamp t to [0,1] for safety
            if (t < 0f) t = 0f;
            else if (t > 1f) t = 1f;

            return new Vector3(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t
            );
        }

        public static Vector3 LerpUnclamped(Vector3 a, Vector3 b, float t)
        {
            return new Vector3(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t
            );
        }

        public static Vector3 Slerp(Vector3 a, Vector3 b, float t)
        {
            // Clamp t for safety
            if (t < 0f) t = 0f;
            else if (t > 1f) t = 1f;

            float magA = a.Length();
            float magB = b.Length();

            // Normalize directions
            Vector3 from = a / magA;
            Vector3 to = b / magB;

            // Dot product clamp
            float dot = Vector3.Dot(from, to);
            dot = Utilities.Clamp(dot, -1f, 1f);

            // If vectors are very close, fall back to Lerp
            if (dot > 0.9995f)
            {
                Vector3 linear = Vector3.Lerp(from, to, t);
                linear = linear.Normalize();
                float mag = magA + (magB - magA) * t;
                return linear * mag;
            }

            // Angle between them
            float theta0 = (float)Math.Acos(dot);  // full angle
            float theta = theta0 * t;              // scaled angle

            float sinTheta0 = (float)Math.Sin(theta0);
            float sinTheta = (float)Math.Sin(theta);

            // Compute orthonormal basis
            Vector3 relative = (to - from * dot).Normalize();

            // Slerp
            Vector3 slerped =
                from * (float)Math.Cos(theta) +
                relative * sinTheta;

            // Interpolate magnitude too (Unity does this)
            float magnitude = magA + (magB - magA) * t;

            return slerped * magnitude;
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
        public static Vector3 Right { get { return new Vector3(-1f, 0f, 0f); } }
        public static Vector3 Up { get { return new Vector3(0f, 1f, 0f); } }
        public static Vector3 Forward { get { return new Vector3(0f, 0f, 1f); } }
    }
}
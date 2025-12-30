using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public struct Vector4
    {
        public float x, y, z, w;

        // --- Constructors -----------------------------------------------------

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public Vector4(float val)
        {
            x = y = z = w = val;
        }

        // --- String -----------------------------------------------------------

        public override string ToString()
        {
            return "(" + x + ", " + y + ", " + z + ", " + w + ")";
        }

        // --- Constants --------------------------------------------------------

        public static readonly Vector4 Zero = new Vector4(0f);
        public static readonly Vector4 One = new Vector4(1f);
        public static readonly Vector4 Right = new Vector4(1f, 0f, 0f, 0f);
        public static readonly Vector4 Up = new Vector4(0f, 1f, 0f, 0f);
        public static readonly Vector4 Forward = new Vector4(0f, 0f, 1f, 0f);
        public static readonly Vector4 Ana = new Vector4(0f, 0f, 0f, 1f); // 4th-dimensional axis

        // --- Operators --------------------------------------------------------

        public static Vector4 operator +(Vector4 a, Vector4 b)
            => new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);

        public static Vector4 operator -(Vector4 a, Vector4 b)
            => new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);

        public static Vector4 operator *(Vector4 v, float f)
            => new Vector4(v.x * f, v.y * f, v.z * f, v.w * f);

        public static Vector4 operator *(float f, Vector4 v)
            => new Vector4(v.x * f, v.y * f, v.z * f, v.w * f);

        public static Vector4 operator /(Vector4 v, float f)
        {
            if (Math.Abs(f) < Utilities.Epsilon)
                throw new DivideByZeroException("Cannot divide Vector4 by zero.");

            return new Vector4(v.x / f, v.y / f, v.z / f, v.w / f);
        }

        public static bool operator ==(Vector4 a, Vector4 b)
            => a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;

        public static bool operator !=(Vector4 a, Vector4 b)
            => !(a == b);

        public override bool Equals(object obj)
        {
            if (obj is Vector4 v)
                return this == v;
            return false;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                int hash = 17;
                hash = hash * 23 + x.GetHashCode();
                hash = hash * 23 + y.GetHashCode();
                hash = hash * 23 + z.GetHashCode();
                hash = hash * 23 + w.GetHashCode();
                return hash;
            }
        }

        // --- Magnitudes -------------------------------------------------------

        public float LengthSquared()
            => x * x + y * y + z * z + w * w;

        public float Length()
            => (float)Math.Sqrt(LengthSquared());

        public float Distance(Vector4 v)
        {
            float dx = v.x - x;
            float dy = v.y - y;
            float dz = v.z - z;
            float dw = v.w - w;
            return (float)Math.Sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
        }

        // --- Normalize --------------------------------------------------------

        public Vector4 Normalize()
        {
            float mag = Length();
            if (mag > Utilities.Epsilon)
                return this / mag;
            return Zero;
        }

        // --- Dot & Projection -------------------------------------------------

        public static float Dot(Vector4 a, Vector4 b)
            => a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

        public static Vector4 Project(Vector4 a, Vector4 b)
        {
            float denom = b.LengthSquared();
            if (denom < Utilities.Epsilon) return Zero;
            return b * (Dot(a, b) / denom);
        }

        public static Vector4 ProjectOnPlane(Vector4 v, Vector4 normal)
            => v - Project(v, normal);

        // --- Lerp / Move / Slerp-ish -----------------------------------------

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            if (t < 0f) t = 0f;
            else if (t > 1f) t = 1f;

            return new Vector4(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t
            );
        }

        public static Vector4 LerpUnclamped(Vector4 a, Vector4 b, float t)
        {
            return new Vector4(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t
            );
        }

        public static Vector4 MoveTowards(Vector4 current, Vector4 target, float maxDelta)
        {
            Vector4 delta = target - current;
            float dist = delta.Length();

            if (dist <= maxDelta || dist == 0f)
                return target;

            return current + delta / dist * maxDelta;
        }

        /// <summary>
        /// Spherical interpolation for directional Vector4 (not quaternion).
        /// </summary>
        public static Vector4 Slerp(Vector4 a, Vector4 b, float t)
        {
            t = Utilities.Clamp(t, 0f, 1f);

            float magA = a.Length();
            float magB = b.Length();

            Vector4 from = a / magA;
            Vector4 to = b / magB;

            float dot = Dot(from, to);
            dot = Utilities.Clamp(dot, -1f, 1f);

            if (dot > 0.9995f)
            {
                Vector4 linear = Lerp(from, to, t).Normalize();
                float mag = magA + (magB - magA) * t;
                return linear * mag;
            }

            float theta0 = (float)Math.Acos(dot);
            float theta = theta0 * t;

            float sin0 = (float)Math.Sin(theta0);
            float sinT = (float)Math.Sin(theta);

            Vector4 relative = (to - from * dot).Normalize();

            Vector4 result =
                from * (float)Math.Cos(theta) +
                relative * sinT;

            float magnitude = magA + (magB - magA) * t;
            return result * magnitude;
        }

        // --- SmoothDamp -------------------------------------------------------

        public static Vector4 SmoothDamp(
            Vector4 current,
            Vector4 target,
            ref Vector4 currentVelocity,
            float smoothTime,
            float maxSpeed,
            float deltaTime)
        {
            smoothTime = Math.Max(0.0001f, smoothTime);

            float omega = 2f / smoothTime;
            float x = omega * deltaTime;
            float exp = 1f / (1f + x + 0.48f * x * x + 0.235f * x * x * x);

            Vector4 change = current - target;

            float maxChange = maxSpeed * smoothTime;
            float changeMag = change.Length();

            if (changeMag > maxChange)
                change = change / changeMag * maxChange;

            Vector4 temp = (currentVelocity + change * omega) * deltaTime;

            currentVelocity = (currentVelocity - temp * omega) * exp;

            Vector4 output = target + (change + temp) * exp;

            Vector4 origToTarget = target - current;
            Vector4 outToTarget = target - output;

            if (Dot(origToTarget, outToTarget) < 0f)
            {
                output = target;
                currentVelocity = Zero;
            }

            return output;
        }
    }
}


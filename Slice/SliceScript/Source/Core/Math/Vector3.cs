using System;
using System.Collections;
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

        public override string ToString()
        {
            return "(" + x.ToString() + ", " + y.ToString() + ", " + z.ToString() + ")";
        }

        public static Vector3 Zero => new Vector3(0.0f);

        public static Vector3 operator -(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
        }
        public static Vector3 operator *(float s, Vector3 v)
        {
            return new Vector3(v.x * s, v.y * s, v.z * s);
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

            ////Dot product -the cosine of the angle between 2 vectors.
            //float dot = Vector3.Dot(a, b);

            //// Clamp it to be in the range of Acos()
            //// This may be unnecessary, but floating point
            //// precision can be a fickle mistress.
            ////Mathf.Clamp(dot, -1.0f, 1.0f);
            //// annotation derHugo: like it stands this is indeed completely unnecessary. 
            //// If something it should be
            //dot = Utilities.Clamp(dot, -1.0f, 1.0f);

            //// Acos(dot) returns the angle between start and end,
            //// And multiplying that by percent returns the angle between
            //// start and the final result.
            //float theta = (float)Math.Acos(dot) * t;
            //Vector3 RelativeVec = b - a * dot;
            //RelativeVec.Normalize();

            //// Orthonormal basis
            //// The final result.
            //return ((a * (float)Math.Cos(theta)) + (RelativeVec * (float)Math.Sin(theta)));
        }

        /// <summary>
        /// Smoothly interpolates a value towards a target using a critically damped spring.
        /// </summary>
        /// <param name="current">Current value.</param>
        /// <param name="target">Target value.</param>
        /// <param name="velocity">Reference to velocity value used internally.</param>
        /// <param name="smoothTime">Approximate time to reach the target.</param>
        /// <param name="deltaTime">Frame delta time.</param>
        /// <returns>The smoothed value.</returns>
        public static Vector3 SmoothDamp(Vector3 current, Vector3 target, ref Vector3 currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
        {
            // Safety
            smoothTime = Math.Max(0.0001f, smoothTime);

            float omega = 2f / smoothTime;

            float x = omega * deltaTime;
            float exp = 1f / (1f + x + 0.48f * x * x + 0.235f * x * x * x);

            // Determine change
            Vector3 change = current - target;

            // Clamp maximum speed
            float maxChange = maxSpeed * smoothTime;
            float changeMag = change.Length();

            if (changeMag > maxChange)
            {
                change = change / changeMag * maxChange;
            }

            Vector3 temp = (currentVelocity + change * omega) * deltaTime;

            // Update velocity
            currentVelocity = (currentVelocity - temp * omega) * exp;

            // Compute output
            Vector3 output = target + (change + temp) * exp;

            // Prevent overshoot
            Vector3 origToTarget = target - current;
            Vector3 outToTarget = target - output;

            if (Vector3.Dot(origToTarget, outToTarget) < 0f)
            {
                output = target;
                currentVelocity = Vector3.Zero;
            }

            return output;
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
        public static Vector3 Right { get { return new Vector3(1f, 0f, 0f); } }
        public static Vector3 Up { get { return new Vector3(0f, 1f, 0f); } }
        public static Vector3 Forward { get { return new Vector3(0f, 0f, 1f); } }
        public static Vector3 RotateTowards(Vector3 from, Vector3 to, float maxDegreesDelta)
        {
            // Step 1: Compute the angle between them
            float dot = Vector3.Dot(to, from);

            // Clamp dot to avoid domain errors due to floating point inaccuracies
            dot = Utilities.Clamp(dot, -1f, 1f);

            // Angle in radians between rotations
            float angle = (float)Math.Acos(dot) * 2f * 180f / (float)Math.PI; // convert to degrees

            // If angle is very small, just return target
            if (angle < 1e-5f)
                return to;

            // Step 2: Determine how much fraction to rotate this step
            float t = Math.Min(1f, maxDegreesDelta / angle);

            // Step 3: Perform spherical interpolation (Slerp)
            return Slerp(from, to, t);
        }

        public static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDelta)
        {
            Vector3 delta = target - current;
            float dist = delta.Length();

            if (dist <= maxDelta || dist == 0f) return target;
            return current + delta / dist * maxDelta;
        }
    }
}
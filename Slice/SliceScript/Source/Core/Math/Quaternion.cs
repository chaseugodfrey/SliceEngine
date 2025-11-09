using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.Remoting.Metadata.W3cXsd2001;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public struct Quaternion
    {
        public float X, Y, Z, W;

        // Identity quaternion
        public static Quaternion Identity => new Quaternion(0, 0, 0, 1);

        public Quaternion(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }

        public Quaternion Conjugate()
        {
            return new Quaternion(-X, -Y, -Z, W);
        }

        public static Vector3 operator *(Quaternion q, Vector3 v)
        {
            // Rotate vector v by quaternion q
            // Formula: v' = q * (0, v) * q⁻¹

            Quaternion vQuat = new Quaternion(v.x, v.y, v.z, 0);

            Quaternion qConjugate = q.Conjugate();

            Quaternion result = q * vQuat * qConjugate;

            return new Vector3(result.X, result.Y, result.Z);
        }

        // Create from axis-angle (axis must be normalized)
        public static Quaternion FromAxisAngle(Vector3 axis, float angleDegrees)
        {
            float rad = angleDegrees * (float)Math.PI / 180f;
            float half = rad / 2f;
            float sin = (float)Math.Sin(half);
            float cos = (float)Math.Cos(half);

            return new Quaternion(
                axis.x * sin,
                axis.y * sin,
                axis.z * sin,
                cos
            );
        }

        // Create from Euler angles (degrees)
        public static Quaternion FromEuler(Vector3 euler)
        {
            float cx = (float)Math.Cos(euler.x * Math.PI / 360f);
            float sx = (float)Math.Sin(euler.x * Math.PI / 360f);
            float cy = (float)Math.Cos(euler.y * Math.PI / 360f);
            float sy = (float)Math.Sin(euler.y * Math.PI / 360f);
            float cz = (float)Math.Cos(euler.z * Math.PI / 360f);
            float sz = (float)Math.Sin(euler.z * Math.PI / 360f);

            return new Quaternion(
                sx * cy * cz - cx * sy * sz,
                cx * sy * cz + sx * cy * sz,
                cx * cy * sz - sx * sy * cz,
                cx * cy * cz + sx * sy * sz
            );
        }

        // Quaternion multiplication (combines rotations)
        public static Quaternion operator *(Quaternion a, Quaternion b)
        {
            return new Quaternion(
                a.W * b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y,
                a.W * b.Y - a.X * b.Z + a.Y * b.W + a.Z * b.X,
                a.W * b.Z + a.X * b.Y - a.Y * b.X + a.Z * b.W,
                a.W * b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z
            );
        }

        // Normalize quaternion
        public void Normalize()
        {
            float mag = (float)Math.Sqrt(X * X + Y * Y + Z * Z + W * W);
            if (mag > 0f)
            {
                X /= mag;
                Y /= mag;
                Z /= mag;
                W /= mag;
            }
        }

        // Convert quaternion to Euler angles (degrees)
        public Vector3 ToEuler()
        {
            Vector3 euler = new Vector3();

            // Roll (X-axis rotation)
            float sinr_cosp = 2f * (W * X + Y * Z);
            float cosr_cosp = 1f - 2f * (X * X + Y * Y);
            euler.x = (float)Math.Atan2(sinr_cosp, cosr_cosp);

            // Pitch (Y-axis rotation)
            float sinp = 2f * (W * Y - Z * X);
            if (Math.Abs(sinp) >= 1f)
                euler.y = (float)(Math.PI / 2f * Math.Sign(sinp));
            else
                euler.y = (float)Math.Asin(sinp);

            // Yaw (Z-axis rotation)
            float siny_cosp = 2f * (W * Z + X * Y);
            float cosy_cosp = 1f - 2f * (Y * Y + Z * Z);
            euler.z = (float)Math.Atan2(siny_cosp, cosy_cosp);

            // Convert to degrees
            euler.x *= 180f / (float)Math.PI;
            euler.y *= 180f / (float)Math.PI;
            euler.z *= 180f / (float)Math.PI;

            return euler;
        }

        public static Quaternion SnapTowards(Quaternion from, Quaternion to)
        {
            return to; // instant snap
        }

        // Example:
        /*
            Quaternion currentRot = transform.Rotation;
            Quaternion targetRot = Quaternion.FromEuler(new Vector3(0, 180, 0));
    
            // Smooth rotation
            transform.Rotation = Quaternion.RotateTowards(currentRot, targetRot, 90f * deltaTime);
    
            // Instant snap (set maxDegreesDelta to a huge number)
            transform.Rotation = Quaternion.RotateTowards(currentRot, targetRot, float.MaxValue);

            // or simply
            transform.Rotation = Quaternion.SnapTowards(currentRot, targetRot);         
        */

        // Rotate from one quaternion toward another by a limited angular speed
        public static Quaternion RotateTowards(Quaternion from, Quaternion to, float maxDegreesDelta)
        {
            // Step 1: Compute the angle between them
            float dot = from.X * to.X + from.Y * to.Y + from.Z * to.Z + from.W * to.W;

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

        public static Quaternion Slerp(Quaternion a, Quaternion b, float t)
        {
            // Clamp t between 0 and 1
            t = Utilities.Clamp(t, 0f, 1f);

            // Compute the cosine of the angle between the quaternions
            float dot = a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;

            // If the dot product is negative, the quaternions have opposite handed-ness
            // and slerp won't take the shorter path. Fix by reversing one quaternion.
            if (dot < 0f)
            {
                b.X = -b.X; b.Y = -b.Y; b.Z = -b.Z; b.W = -b.W;
                dot = -dot;
            }

            const float DOT_THRESHOLD = 0.9995f;
            if (dot > DOT_THRESHOLD)
            {
                // Very close — use linear interpolation to avoid division by 0
                Quaternion result = new Quaternion(
                    a.X + t * (b.X - a.X),
                    a.Y + t * (b.Y - a.Y),
                    a.Z + t * (b.Z - a.Z),
                    a.W + t * (b.W - a.W)
                );
                result.Normalize();
                return result;
            }

            // Calculate angle between quaternions
            float theta_0 = (float)Math.Acos(dot); // angle between input quaternions
            float theta = theta_0 * t;             // angle to interpolate to

            float sin_theta = (float)Math.Sin(theta);
            float sin_theta_0 = (float)Math.Sin(theta_0);

            float s0 = (float)Math.Cos(theta) - dot * sin_theta / sin_theta_0;
            float s1 = sin_theta / sin_theta_0;

            Quaternion q = new Quaternion(
                (a.X * s0) + (b.X * s1),
                (a.Y * s0) + (b.Y * s1),
                (a.Z * s0) + (b.Z * s1),
                (a.W * s0) + (b.W * s1)
            );
            q.Normalize();
            return q;
        }

        // Create a quaternion that looks in the 'forward' direction, with optional 'up' reference
        public static Quaternion LookRotation(Vector3 forward, Vector3 up)
        {
            forward = forward.Normalize();
            up = up.Normalize();

            // Recompute orthogonal basis
            Vector3 right = Vector3.Cross(up, forward).Normalize();
            up = Vector3.Cross(forward, right);

            // Build rotation matrix
            float m00 = right.x, m01 = up.x, m02 = forward.x;
            float m10 = right.y, m11 = up.y, m12 = forward.y;
            float m20 = right.z, m21 = up.z, m22 = forward.z;

            float trace = m00 + m11 + m22;

            Quaternion q = new Quaternion();

            if (trace > 0f)
            {
                float s = (float)Math.Sqrt(trace + 1f) * 2f;
                q.W = 0.25f * s;
                q.X = (m21 - m12) / s;
                q.Y = (m02 - m20) / s;
                q.Z = (m10 - m01) / s;
            }
            else if ((m00 > m11) && (m00 > m22))
            {
                float s = (float)Math.Sqrt(1f + m00 - m11 - m22) * 2f;
                q.W = (m21 - m12) / s;
                q.X = 0.25f * s;
                q.Y = (m01 + m10) / s;
                q.Z = (m02 + m20) / s;
            }
            else if (m11 > m22)
            {
                float s = (float)Math.Sqrt(1f + m11 - m00 - m22) * 2f;
                q.W = (m02 - m20) / s;
                q.X = (m01 + m10) / s;
                q.Y = 0.25f * s;
                q.Z = (m12 + m21) / s;
            }
            else
            {
                float s = (float)Math.Sqrt(1f + m22 - m00 - m11) * 2f;
                q.W = (m10 - m01) / s;
                q.X = (m02 + m20) / s;
                q.Y = (m12 + m21) / s;
                q.Z = 0.25f * s;
            }

            q.Normalize();
            return q;
        }

        // Overload with only forward vector (uses global up)
        public static Quaternion LookRotation(Vector3 forward)
        {
            return LookRotation(forward, new Vector3(0, 1, 0));
        }
    }
}

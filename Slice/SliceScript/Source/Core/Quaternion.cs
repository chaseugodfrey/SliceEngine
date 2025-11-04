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
    }
}

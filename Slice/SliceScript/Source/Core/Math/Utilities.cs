using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    /// <summary>
    /// A collection of math and helper utility functions for SliceEngine.
    /// </summary>
    public struct Utilities
    {
        /// <summary>
        /// Smallest positive float value (approx. zero).
        /// </summary>
        public const float Epsilon = 1e-6f;

        /// <summary>
        /// Smoothly interpolates a value towards a target using a critically damped spring.
        /// </summary>
        /// <param name="current">Current value.</param>
        /// <param name="target">Target value.</param>
        /// <param name="velocity">Reference to velocity value used internally.</param>
        /// <param name="smoothTime">Approximate time to reach the target.</param>
        /// <param name="deltaTime">Frame delta time.</param>
        /// <returns>The smoothed value.</returns>
        public static float SmoothDamp(float current, float target, ref float velocity, float smoothTime, float deltaTime)
        {
            float omega = 2f / smoothTime;
            float x = omega * deltaTime;
            float exp = 1f / (1f + x + 0.48f * x * x + 0.235f * x * x * x);
            float change = current - target;
            float temp = (velocity + omega * change) * deltaTime;
            velocity = (velocity - omega * temp) * exp;
            return target + (change + temp) * exp;
        }

        /// <summary>
        /// Performs a Hermite-smoothed interpolation between two values.
        /// </summary>
        /// <param name="from">Start value.</param>
        /// <param name="to">End value.</param>
        /// <param name="t">Interpolation factor between 0 and 1.</param>
        /// <returns>Interpolated value.</returns>
        public static float SmoothStep(float from, float to, float t)
        {
            // Clamp t to [0, 1]
            if (t < 0f) t = 0f;
            else if (t > 1f) t = 1f;

            // Hermite smoothing
            t = t * t * (3f - 2f * t);

            return from + (to - from) * t;
        }

        public static bool AreAllPointsCollinear3D(Vector3[] points)
        {
            if (points.Length < 3) return true;

            Vector3 p1 = points[0];
            Vector3 p2 = points[1];

            // Manually calculating the base vector (p2 - p1)
            float baseVx = p2.x - p1.x;
            float baseVy = p2.y - p1.y;
            float baseVz = p2.z - p1.z;

            float epsilon = 0.0001f;

            for (int i = 2; i < points.Length; i++)
            {
                Vector3 pi = points[i];

                // Manually calculating the check vector (pi - p1)
                float checkVx = pi.x - p1.x;
                float checkVy = pi.y - p1.y;
                float checkVz = pi.z - p1.z;

                // 3D Cross Product
                float crossX = (baseVy * checkVz) - (baseVz * checkVy);
                float crossY = (baseVz * checkVx) - (baseVx * checkVz);
                float crossZ = (baseVx * checkVy) - (baseVy * checkVx);

                // Check the squared magnitude of the cross product vector
                float sqrMagnitude = (crossX * crossX) + (crossY * crossY) + (crossZ * crossZ);

                if (sqrMagnitude > epsilon)
                {
                    return false; // Not a straight line
                }
            }
            return true; // All points passed the check
        }

        public static Vector3 GetShapeCenter(Vector3[] shapePoints, int numberOfCorners)
        {
            Vector3 sum = Vector3.Zero;

            // Loop through only the points that make up the shape 
            // (ignoring the final return trip to the original camera position)
            for (int i = 0; i < numberOfCorners; i++)
            {
                sum += shapePoints[i];
            }

            return new Vector3(sum.x / numberOfCorners, sum.y / numberOfCorners, sum.z / numberOfCorners);
        }

        /// <summary>
        /// Clamps a value between a minimum and maximum limit.
        /// </summary>
        public static T Clamp<T>(T value, T min, T max) where T : IComparable<T>
        {            
            if (value.CompareTo(min) < 0) return min;
            if (value.CompareTo(max) > 0) return max;
            return value;
        }

        /// <summary>
        /// Linearly interpolates between two values.
        /// </summary>
        public static float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            return new Vector3(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t));
        }

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            return new Vector4(
                Lerp(a.x, b.x, t),
                Lerp(a.y, b.y, t),
                Lerp(a.z, b.z, t),
                Lerp(a.w, b.w, t)
            );
        }

        public static Vector3 FromHSV(float h, float s, float v)
        {
            float r = 0, g = 0, b = 0;

            float i = (float)Math.Floor(h * 6.0f);
            float f = h * 6.0f - i;

            float p = v * (1.0f - s);
            float q = v * (1.0f - f * s);
            float t = v * (1.0f - (1.0f - f) * s);

            int mod = (int)i % 6;

            switch (mod)
            {
                case 0: r = v; g = t; b = p; break;
                case 1: r = q; g = v; b = p; break;
                case 2: r = p; g = v; b = t; break;
                case 3: r = p; g = q; b = v; break;
                case 4: r = t; g = p; b = v; break;
                case 5: r = v; g = p; b = q; break;
            }

            return new Vector3(r, g, b);
        }
        /// <summary>
        /// Calculates the normalized interpolation factor between two values.
        /// </summary>
        public static float InverseLerp(float a, float b, float value)
        {
            if (a == b) return 0f;
            return Clamp((value - a) / (b - a), 0f, 1f);
        }

        /// <summary>
        /// Applies an ease-in curve (quadratic).
        /// </summary>
        public static float EaseIn(float t) => t * t;

        /// <summary>
        /// Applies an ease-out curve (quadratic).
        /// </summary>
        public static float EaseOut(float t) => 1 - (1 - t) * (1 - t);

        /// <summary>
        /// Applies a quadratic ease-in-out curve.
        /// </summary>
        public static float EaseInOut(float t)
        {
            return (t < 0.5f)
                ? 2f * t * t
                : 1f - (float)Math.Pow(-2f * t + 2f, 2f) / 2f;
        }

        /// <summary>
        /// Interpolates between two angles while handling wrap-around beyond 360 degrees.
        /// </summary>
        public static float LerpAngle(float a, float b, float t)
        {
            float delta = Repeat((b - a), 360f);
            if (delta > 180f) delta -= 360f;
            return a + delta * Clamp(t, 0f, 1f);
        }

        /// <summary>
        /// Loops a value within a repeating length.
        /// </summary>
        public static float Repeat(float value, float length)
        {
            return value - (float)Math.Floor(value / length) * length;
        }

        /// <summary>
        /// Calculates the shortest signed angle difference between two angles.
        /// </summary>
        public static float DeltaAngle(float a, float b)
        {
            float delta = Repeat((b - a), 360f);
            if (delta > 180f) delta -= 360f;
            return delta;
        }

        /// <summary>
        /// Returns the min value of a comparable type.
        /// </summary>
        public static T Min<T>(T a, T b) where T : IComparable<T>
        {
            return a.CompareTo(b) < 0 ? a : b;
        }

        /// <summary>
        /// Returns the max value of a comparable type.
        /// </summary>
        public static T Max<T>(T a, T b) where T : IComparable<T>
        {            
            return a.CompareTo(b) > 0 ? a : b;
        }

        /// <summary>
        /// Returns sine of a value (in radians).
        /// </summary>
        public static float Sin(float value)
        {
            return (float)Math.Sin(value);
        }

        /// <summary>
        /// Returns sine of a value (in degrees).
        /// </summary>
        public static float SinDeg(float degrees)
        {
            return (float)Math.Sin(Deg2Rad(degrees));
        }

        /// <summary>
        /// Converts degrees to radians.
        /// </summary>
        public static float Deg2Rad(float degrees)
        {
            return degrees * (float)Math.PI / 180f;
        }

        /// <summary>
        /// Converts radians to degrees.
        /// </summary>
        public static float Rad2Deg(float radians)
        {
            return radians * 180f / (float)Math.PI;
        }

        /// <summary>
        /// Ping-pongs a value back and forth between 0 and length.
        /// </summary>
        public static float PingPong(float t, float length)
        {
            return length - Math.Abs((t % (2 * length)) - length);
        }

        /// <summary>
        /// Remaps a value from one range into another.
        /// </summary>
        public static float Remap(float value, float from1, float to1, float from2, float to2)
        {
            return from2 + (value - from1) * (to2 - from2) / (to1 - from1);
        }

        /// <summary>
        /// Computes 2D distance between two points using Vector2s.
        /// When comparing distance, please use the squared version to prevent using sqrt.
        /// </summary>
        public static float Distance2D(Vector2 a, Vector2 b)
        {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            return (float)Math.Sqrt(dx * dx + dy * dy);
        }

        /// <summary>
        /// Computes the squared 2D distance between two points using Vector2s.
        /// Eg: Comparing distance between 2 points.
        /// </summary>
        public static float Distance2DSquared(Vector2 a, Vector2 b)
        {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            return (float)(dx * dx + dy * dy);
        }

        /// <summary>
        /// Computes 2D distance between two points on the XZ plane.
        /// When comparing distance, please use the squared version to prevent using sqrt.
        /// </summary>
        public static float Distance2D(Vector3 a, Vector3 b)
        {
            float dx = a.x - b.x;
            float dz = a.z - b.z;
            return (float)Math.Sqrt(dx * dx + dz * dz);
        }

        /// <summary>
        /// Computes the squared 2D distance between two points on the XZ plane.
        /// Eg: Comparing distance between 2 points.
        /// </summary>
        public static float Distance2DSquared(Vector3 a, Vector3 b)
        {
            float dx = a.x - b.x;
            float dz = a.z - b.z;
            return (float)(dx * dx + dz * dz);
        }

        /// <summary>
        /// Computes 3D distance between two points.
        /// When comparing distance, please use the squared version to prevent using sqrt.
        /// </summary>
        public static float Distance3D(Vector3 a, Vector3 b)
        {
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float dz = b.z - a.z;
            return (float)Math.Sqrt(dx * dx + dy * dy + dz * dz);
        }

        /// <summary>
        /// Computes squared 3D distance between two points (to save cost from using sqrt).
        /// Eg: Comparing distance between 2 points.
        /// </summary>
        public static float Distance3DSquared(Vector3 a, Vector3 b)
        {
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float dz = b.z - a.z;
            return (float)(dx * dx + dy * dy + dz * dz);
        }

        /// <summary>
        /// Returns a random point inside a sphere of a given radius.
        /// </summary>
        public static Vector3 RandomInsideSphere(float radius)
        {
            return new Vector3(
                SliceRandom.ValueFloat() * 2f - 1f,
                SliceRandom.ValueFloat() * 2f - 1f,
                SliceRandom.ValueFloat() * 2f - 1f
            ).Normalize() * radius;
        }

        /// <summary>
        /// Computes a quadratic Bezier curve interpolation.
        /// </summary>
        public static Vector3 Bezier(Vector3 a, Vector3 b, Vector3 c, float t)
        {
            float u = 1f - t;
            return u * u * a + 2 * u * t * b + t * t * c;
        }

        /// <summary>
        /// Computes a parabolic arc between two points.
        /// </summary>
        static Vector3 Parabola(Vector3 start, Vector3 end, float height, float t)
        {
            // Linear interpolation
            Vector3 pos = Vector3.Lerp(start, end, t);

            // Add parabolic height using quadratic formula
            float parabola = 4f * height * t * (1f - t);

            pos.y += parabola;
            return pos;
        }

        /// <summary>
        /// Coroutine that animates a transform along a parabolic arc from start to end.
        /// </summary>
        /// <param name="transform">Transform to move.</param>
        /// <param name="start">Starting position.</param>
        /// <param name="end">Ending position.</param>
        /// <param name="height">Peak height of the parabola.</param>
        /// <param name="duration">Time to complete the arc.</param>
        /// <returns>Coroutine enumerator.</returns>
        public static IEnumerator ParabolaCoroutine(Transform transform, Vector3 start, Vector3 end, float height, float duration)
        {
            float time = 0f;

            while (time < duration)
            {
                float t = time / duration;
                transform.Position = Parabola(start, end, height, t);
                time += Time.deltaTime;               
                yield return null;
            }

            // Ensure final position exactly matches end
            transform.Position = end;
        }
    }
}

/* File Documentation ----------------------------------------------------------------------------- 
file:           Vector2.cs

author:		    Gideon Francis (100%)

email:			g.francis@digipen.edu

brief:          This file defines the `Vector2` struct, a 2D vector with basic vector operations 
                such as normalization, subtraction, and distance calculation. Provides utility for 
                2D mathematical operations within the Carmicah engine.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
--------------------------------------------------------------------------------------------------*/


using System;

namespace SliceEngine
{
    public struct Vector2
    {
        public float x, y;

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public Vector2(float val)
        {
            this.x = val;
            this.y = val;
        }

        // uncomment the Equals function to resolve  
        public override bool Equals(object o)
        {
            return true;
        }

        public override int GetHashCode()
        {
            return 0;
        }

        public static Vector2 Zero => new Vector2(0.0f, 0.0f);

        public static Vector2 operator -(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x - v2.x, v1.y - v2.y);
        }

        public static Vector2 operator -(Vector2 v)
        {
            return new Vector2(-v.x, -v.y);
        }

        public static Vector2 operator *(Vector2 v1, float f1)
        {
            return new Vector2(v1.x * f1, v1.y * f1);
        }
        public static Vector2 operator *(float f, Vector2 v)
        {
            return new Vector2(v.x * f, v.y * f);
        }
        public static Vector2 operator /(Vector2 v, float f)
        {
            return new Vector2(v.x / f, v.y / f);
        }

        public static Vector2 operator +(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x + v2.x, v1.y + v2.y);
        }

        public static bool operator ==(Vector2 v1, Vector2 v2)
        {
            return v1.x == v2.x && v1.y == v2.y;
        }

        public static bool operator !=(Vector2 v1, Vector2 v2)
        {
            return !(v1 == v2);
        }

        public Vector2 Normalize()
        {
            float magnitude = (float)Math.Sqrt(x * x + y * y);
            if (magnitude > 0)
            {
                return new Vector2(x / magnitude, y / magnitude);
            }
            else
            {
                return Vector2.Zero;
            }
        }

        public float Distance(Vector2 other)
        {
            float dx = other.x - x;
            float dy = other.y - y;
            return (float)Math.Sqrt(dx * dx + dy * dy);
        }

        public static Vector2 Lerp(Vector2 end, Vector2 start, float t)
        {
            t = Clamp(t);
            return new Vector2(
                    end.x + (start.x - end.x) * t,
                    end.y + (start.y - end.y) * t
                );
        }

        private static float Clamp(float value)
        {
            if (value < 0) return 0;
            if (value > 1) return 1;

            return value;
        }
    }
}

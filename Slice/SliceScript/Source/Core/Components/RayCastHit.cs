using System;

namespace SliceEngine
{
    public struct RayCastHit
    {
        //disatnce, point, transformtt main ones for now
        public float distance;
        public Vector3 point;
        public Vector3 normal;
        public Transform transform;

        // migth add collider, rigidbody, normal in the future if requested other will need further research

    }


    public struct Ray
    {
        public Vector3 direction;
        public Vector3 origin;

        public Ray(Vector3 dir, Vector3 ori)
        {
            direction = dir;
            origin = ori;
        }


        public Vector3 GetPoint(float distance)
        {
            return origin + (direction * distance);
        }

        public override String ToString() 
        {
            return "Origin: " + origin.ToString() + " Direction: " + direction.ToString();
        }

    }
}
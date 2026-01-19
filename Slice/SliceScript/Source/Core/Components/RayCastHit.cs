using System;

namespace SliceEngine
{
    public struct RayCastHit
    {
        //disatnce, point, transformtt main ones for now
        float distance;
        Vector3 point;
        Transform transform;

        // migth add collider, rigidbody, normal in the future if requested other will need further research

    }
}
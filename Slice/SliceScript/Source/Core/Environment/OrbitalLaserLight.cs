using SliceEngine;
using System;

namespace SliceEngine
{
    public class OrbitalLaserLight : SliceBehaviour
    {
        public float moveSpeed = 5f;

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            Vector3 currPos = this.GetComponent<Transform>().Position;

            Vector3 direction = new Vector3(0,-1, 0);

            Vector3 move = direction.Normalize() * moveSpeed * dt;  

            this.GetComponent<Transform>().Position = currPos + move;

        }

    }
}
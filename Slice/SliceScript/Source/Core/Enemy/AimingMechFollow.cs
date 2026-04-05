using SliceEngine;
using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class AimingMechFollow : SliceBehaviour
    {
        public Transform target;
        public Vector3 destination;
        public Vector3 offset;
        public bool followTarget;
        public float followTightness = 0.0f;
        public float followRange = 200.0f;
        public float followOscillator = 0.0f;
        public float followOscillatorRate = 0.5f;
        public float followOscillatorTiming = 2.5f;
        public float followExternalModifier = 1.0f;

        public override void OnUpdate(float dt)
        {

            FollowTarget(dt);
            //projectileSM.OnUpdate(dt);
        }

        public void SetTarget(Transform targetTransform = null, float speedModifier = 1.0f)
        {
            target = targetTransform;
            followExternalModifier = speedModifier;
        }

        void FollowTarget(float dt)
        {
            if (followTarget)
            {
                if (target != null)
                {
                    followOscillator += followOscillatorRate * dt;
                    if (followOscillator > followOscillatorTiming || followOscillator <= 0.0f)
                    {
                        followOscillatorRate *= -1.0f;
                    }

                    float distance = Utilities.Distance3D(transform.Position, destination);
                    float tightness = Utilities.Clamp(distance / followRange, 0.0f, 1.0f) - followTightness;
                    followTightness += tightness * 0.5f;
                    destination = target.WorldPosition + offset;

                    Vector3 direction = (destination - transform.Position).Normalize();
                    transform.Position = transform.Position + direction * followTightness * dt * 100f * followOscillator * followExternalModifier;

                }
            }
        }

        public void DestroyChildren()
        {
            foreach (var child in gameObject.GetAllChildren())
            {
                child.Destroy();
            }
        }
    }
}
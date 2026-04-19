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

        // When true, offset is treated as camera-local space so mechs stay in the player's view
        public bool useCameraRelativeOffset = false;

        public override void OnUpdate(float dt)
        {
            FollowTarget(dt);
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

                    Vector3 worldOffset = offset;
                    if (useCameraRelativeOffset)
                        worldOffset = CameraRelativeOffset(offset);

                    destination = target.WorldPosition + worldOffset;

                    Vector3 direction = (destination - transform.Position).Normalize();
                    transform.Position = transform.Position + direction * followTightness * dt * 100f * followOscillator * followExternalModifier;
                }
            }
        }

        // Rotates a local-space offset into world space using the camera's facing direction,
        // keeping mechs within the player's view.
        private Vector3 CameraRelativeOffset(Vector3 localOffset)
        {
            Vector3 forward = Bootstrap.CameraController.transform.Forward;
            forward.y = 0;
            float mag = forward.Magnitude();
            if (mag > 0.001f) forward = forward / mag;

            Vector3 right = new Vector3(forward.z, 0, -forward.x);

            return right * localOffset.x + new Vector3(0, localOffset.y, 0) + forward * localOffset.z;
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

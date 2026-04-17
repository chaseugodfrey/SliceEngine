using SliceEngine;
using System;

namespace SliceEngine
{
    public class MovingPlatform : SliceBehaviour
    {
        // World-space offset the platform travels from its starting position
        public Vector3 moveOffset = new Vector3(5f, 0f, 0f);

        // Seconds to travel between the two endpoints
        public float moveDuration = 2f;

        // Seconds spent easing in/out at each end of a movement leg.
        // The platform reaches full speed after easeTime and begins slowing down easeTime before stopping.
        public float easeTime = 0.1f;

        // Seconds to pause at each end before reversing
        public float pauseAtStart = 0.5f;
        public float pauseAtEnd = 0.5f;

        // Loop back and forth indefinitely; if false, stops after one round trip
        public bool loop = true;

        // Begin moving immediately on scene start; if false, starts on first player contact
        public bool startImmediately = true;

        private Transform platformTransform;
        private AudioSource audioSource;
        private float originalVolume = 1f;

        private Vector3 origin;
        private Vector3 prevPosition;
        private Vector3 platformVelocity;

        private float timer = 0f;
        private bool isMoving = false;
        private bool isPlayerOnPlatform = false;

        public override void OnCreate()
        {
            platformTransform = GetComponent<Transform>();
            origin = platformTransform.Position;
            prevPosition = origin;

            if (HasComponent<AudioSource>())
            {
                audioSource = GetComponent<AudioSource>();
                originalVolume = audioSource.Volume;
                audioSource.SpatialBlend = 1f;
                audioSource.IsLoop = true;
                audioSource.Volume = 0f;
                audioSource.Play();
            }

            isMoving = startImmediately;
        }

        public override void OnUpdate(float dt)
        {
            Vector3 posBeforeMove = platformTransform.Position;
            float volumeFactor = 0f;

            if (isMoving)
            {
                timer += dt;

                float cycleLength = moveDuration + pauseAtEnd + moveDuration + pauseAtStart;
                float cycleTime = loop ? timer % cycleLength : Math.Min(timer, cycleLength);

                if (!loop && timer >= cycleLength)
                {
                    platformTransform.Position = origin;
                    isMoving = false;
                }
                else
                {
                    platformTransform.Position = EvaluatePosition(cycleTime);
                    volumeFactor = ComputeVolumeFactor(cycleTime);
                }
            }

            if (audioSource != null)
                audioSource.Volume = originalVolume * volumeFactor;

            // Velocity from position delta — used to carry the player
            if (dt > 0f)
                platformVelocity = (platformTransform.Position - posBeforeMove) / dt;
            else
                platformVelocity = Vector3.Zero;

            prevPosition = platformTransform.Position;

            // Only inject horizontal velocity — Jolt contact resolution already carries
            // the player vertically when the platform moves up/down. Adding Y on top
            // causes the physics solver to fight itself, producing jitter and sideways ejection.
            if (isPlayerOnPlatform && Bootstrap.Player != null)
            {
                Vector3 horizontalVelocity = new Vector3(platformVelocity.x, 0f, platformVelocity.z);
                Bootstrap.Player.AddVelocityModifier(horizontalVelocity);
            }
        }

        // Trapezoid position curve: quadratic ease-in for easeTime seconds, linear at full speed,
        // quadratic ease-out for easeTime seconds. Gives quick acceleration/deceleration with a
        // sustained constant-speed middle section, unlike SmoothStep which eases the entire leg.
        private float EvaluateLegT(float t)
        {
            float et = Math.Min(easeTime, moveDuration * 0.45f);
            float d = et / (2f * (moveDuration - et));   // position fraction covered during ease-in
            float vMax = 1f / (moveDuration - et);        // normalized peak velocity

            if (t <= et)
            {
                float s = t / et;
                return d * s * s;
            }
            else if (t >= moveDuration - et)
            {
                float s = (moveDuration - t) / et;
                return 1f - d * s * s;
            }
            else
            {
                return d + vMax * (t - et);
            }
        }

        // Normalized velocity of the trapezoid profile: ramps linearly 0→1 over easeTime,
        // holds at 1 during the middle, ramps 1→0 over the final easeTime.
        // Used to drive audio volume so it matches the platform's acceleration exactly.
        private float LegVelocityFactor(float t)
        {
            float et = Math.Min(easeTime, moveDuration * 0.45f);

            if (t <= et)
                return t / et;
            else if (t >= moveDuration - et)
                return (moveDuration - t) / et;
            else
                return 1f;
        }

        private Vector3 EvaluatePosition(float cycleTime)
        {
            float cursor = 0f;

            // Leg 1: origin → origin + offset
            if (cycleTime < cursor + moveDuration)
            {
                float t = EvaluateLegT(cycleTime - cursor);
                return Utilities.Lerp(origin, origin + moveOffset, t);
            }
            cursor += moveDuration;

            // Pause at end position
            if (cycleTime < cursor + pauseAtEnd)
                return origin + moveOffset;
            cursor += pauseAtEnd;

            // Leg 2: origin + offset → origin
            if (cycleTime < cursor + moveDuration)
            {
                float t = EvaluateLegT(cycleTime - cursor);
                return Utilities.Lerp(origin + moveOffset, origin, t);
            }

            // Pause at start position
            return origin;
        }

        private float ComputeVolumeFactor(float cycleTime)
        {
            float cursor = 0f;

            // Leg 1
            if (cycleTime < cursor + moveDuration)
                return LegVelocityFactor(cycleTime - cursor);
            cursor += moveDuration;

            // Pause at end
            if (cycleTime < cursor + pauseAtEnd) return 0f;
            cursor += pauseAtEnd;

            // Leg 2
            if (cycleTime < cursor + moveDuration)
                return LegVelocityFactor(cycleTime - cursor);

            // Pause at start
            return 0f;
        }

        // Called by PlatformCollisionRelay scripts on child objects
        public void NotifyPlayerEnter()
        {
            isPlayerOnPlatform = true;
            if (!startImmediately)
                isMoving = true;
        }

        public void NotifyPlayerStay()
        {
            isPlayerOnPlatform = true;
        }

        public void NotifyPlayerExit()
        {
            isPlayerOnPlatform = false;
            if (Bootstrap.Player != null)
                Bootstrap.Player.AddVelocityModifier(Vector3.Zero);
        }

        public override void OnCollideEnter(uint other)
        {
            if (Bootstrap.Player == null) return;

            if (other == Bootstrap.Player.gameObject.mID)
                NotifyPlayerEnter();
        }

        public override void OnCollideStay(uint other)
        {
            if (Bootstrap.Player != null && other == Bootstrap.Player.gameObject.mID)
                NotifyPlayerStay();
        }

        public override void OnCollideExit(uint other)
        {
            if (Bootstrap.Player == null) return;

            if (other == Bootstrap.Player.gameObject.mID)
                NotifyPlayerExit();
        }
    }
}

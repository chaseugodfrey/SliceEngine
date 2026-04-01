using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class CollapsingPlatform : SliceBehaviour
    {
        public GameObject platformParent;
        public GameObject player;
        Transform platformTransform;

        float initialY;
        float collapseHeight = 0.01f;

        public float timeDelayBeforeCollapsing = 1f;
        public float timeToCollapse = 0.25f;
        public float timeToRebuild = 2f;

        bool isStable = true;
        float timer;

        enum PlatformState
        {
            Stable,
            Triggered,
            Collapsing,
            Rising
        }

        PlatformState state = PlatformState.Stable;

        public override void OnAwake()
        {
            platformTransform = platformParent.GetComponent<Transform>();
            initialY = platformTransform.transform.Scale.y;
            state = PlatformState.Stable;
        }

        public override void OnTriggerEnter(uint other)
        {
            SliceLog.Console($"Trigger entered by object with ID: {other}");
            if (other == player.mID && state == PlatformState.Stable)
            {
                state = PlatformState.Triggered;
                timer = timeDelayBeforeCollapsing;
                SliceLog.Console("Player entered collapsing platform trigger, starting timer.");
            }
        }

        void RunState()
        {
            switch (state)
            {
                case PlatformState.Stable:
                    break;
                case PlatformState.Triggered:
                    TriggerCollapse();
                    break;
                case PlatformState.Collapsing:
                    Collapse();
                    break;
                case PlatformState.Rising:
                    Rise();
                    break;

            }
        }

        void TriggerCollapse()
        {
            if (timer > 0)
                return;

            SliceLog.Console("PLATFORM TRIGGERED");
            state = PlatformState.Collapsing;
            timer = timeToCollapse;
        }

        void Collapse()
        {
            if (timer <= 0.0f)
            {
                platformTransform.Scale = new Vector3(platformTransform.Scale.x, collapseHeight, platformTransform.Scale.z);
                state = PlatformState.Rising;
                timer = timeToRebuild;
                SliceLog.Console("PLATFORM COLLAPSED");
            }

            else
            {
                float height = Utilities.Lerp(collapseHeight, initialY, timer / timeToCollapse);
                platformTransform.Scale = new Vector3(platformTransform.Scale.x, height, platformTransform.Scale.z);
            }
        }

        void Rise()
        {
            if (timer <= 0.0f)
            {
                state = PlatformState.Stable;
                timer = 0;
                platformTransform.Scale = new Vector3(platformTransform.Scale.x, initialY, platformTransform.Scale.z);
                SliceLog.Console("PLATFORM RAISED");
            }

            else
            {
                //SliceLog.Console($"Rising platform, time remaining: {timer}");
                float height = Utilities.Lerp(initialY, collapseHeight, timer / timeToRebuild);
                platformTransform.Scale = new Vector3(platformTransform.Scale.x, height, platformTransform.Scale.z);
            }
        }

        public override void OnUpdate(float dt)
        {
            if (state != PlatformState.Stable)
            {
                timer -= Time.deltaTime;
                RunState();
            }
        }
    }
}
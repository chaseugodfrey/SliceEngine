using SliceEngine;

namespace SliceEngine
{
    // Attach this to any child object with a collider.
    // Assign platformParent to the GameObject that holds the MovingPlatform script.
    public class PlatformCollisionRelay : SliceBehaviour
    {
        public GameObject platformParent;

        private MovingPlatform platform;

        public override void OnCreate()
        {
            platform = platformParent?.As<MovingPlatform>();

            if (platform == null)
                SliceLog.Warn("PlatformCollisionRelay: could not find MovingPlatform on platformParent.");
        }

        public override void OnCollideEnter(uint other)
        {
            if (platform == null || Bootstrap.Player == null) return;

            if (other == Bootstrap.Player.gameObject.mID)
                platform.NotifyPlayerEnter();
        }

        public override void OnCollideStay(uint other)
        {
            if (platform == null || Bootstrap.Player == null) return;

            if (other == Bootstrap.Player.gameObject.mID)
                platform.NotifyPlayerStay();
        }

        public override void OnCollideExit(uint other)
        {
            if (platform == null || Bootstrap.Player == null) return;

            if (other == Bootstrap.Player.gameObject.mID)
                platform.NotifyPlayerExit();
        }
    }
}

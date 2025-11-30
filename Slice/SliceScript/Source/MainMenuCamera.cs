using SliceEngine;
using System;


namespace SliceEngine
{
    public class MainMenuCamera : SliceBehaviour, IInitializable
    {
        public float totalAnimTime = 5.0f;

        private bool isActivated = false;
        private Vector3 startCamPosition;
        private Quaternion startCamRot;
        private Vector3 finalCamPosition;
        private Quaternion finalCamRot;
        private float timeSinceActivated;
        public override void OnCreate()
        {
            Camera.SetMainCamera(this.gameObject);
        }

        public void ActivateMainMenuCamera()
        {
            isActivated = true;
            timeSinceActivated = 0.0f;
            startCamPosition = transform.Position;
            startCamRot = transform.RotationQuat;
            finalCamPosition = gameObject.FindGameObjectWithName("Camera").GetComponent<Transform>().Position;
            finalCamRot = gameObject.FindGameObjectWithName("Camera").GetComponent<Transform>().RotationQuat;
        }

        public void Initialize()
        {
        }
        public override void OnUpdate(float dt)
        {
            if (isActivated)
            {
                timeSinceActivated += dt;
                if (timeSinceActivated >= totalAnimTime)
                {
                    Camera.SetMainCamera(gameObject.FindGameObjectWithName("Camera"));
                    Destroy();
                }
                else
                {
                    float t = Utilities.EaseIn(timeSinceActivated / totalAnimTime);

                    transform.Position = Utilities.Lerp(startCamPosition, finalCamPosition, t);
                    transform.RotationQuat = Quaternion.Slerp(startCamRot, finalCamRot, t);
                }
            }
        }
    }
}
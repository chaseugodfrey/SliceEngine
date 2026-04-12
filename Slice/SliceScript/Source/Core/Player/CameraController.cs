using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;


namespace SliceEngine
{
    public class CameraController : SliceBehaviour, IInitializable
    {
        public Vector3 sensitivity = new Vector3(10f, 8f, 0f);
        public Vector3 yClamp = new Vector3(-40f, 60f, 0f);
        public List<GameObject> cameraWayPoints = new List<GameObject>();
        public float resolutionX = 1920, resolutionY = 1080;
        public bool LockCamera = false;
        private float pitch = 0f;
        private Vector3 originalCameraPosition;
        public float rectangleWidth = 10f;
        public float cameraRotationSpeed = 10.0f;
        public float cameraTransitionSpeed = 5.0f;
        public float shakeMagnitude = 0.5f;
        public float shakeDuration = 0.2f;

        private Vector3 shakeOffset = Vector3.Zero;
        private Quaternion actual_cam_quat = Quaternion.Identity;

        public GameObject cameraChild;
        public Camera cam;
        public float collisionCheckOffset = 1f;
        private Vector3 defaultCameraOffset = new Vector3(0f, 0f, 0f);
        private float defaultCameraOffsetDist = 0f;
        public Vector3 cameraOffset = new Vector3(0f, 2f, -5f); // 2 units up, 5 units back 
        public float smoothFollowSpeed = 10f;

        public GameObject followTarget;
        public Vector3 followOffset = new Vector3(0f, 2f, -5f);
        public float followPositionSmoothness = 5f;
        public float followRotationSmoothness = 5f;
        //private Vector2 lastMousePos;

        public float collisionRadius = 0.25f;


        public override void OnCreate()
        {
            //SliceLog.Log("Rotation: x<" + transform.Rotation.x + ">y<" + transform.Rotation.y + ">z<" + transform.Rotation.z);
            Camera.SetMainCamera(cameraChild);
            cam = cameraChild.GetComponent<Camera>();
        }

        public void Initialize()
        {
            //Console.WriteLine("CameraCont Ini called");

            defaultCameraOffset = cameraChild.GetComponent<Transform>().Position;
            defaultCameraOffsetDist = defaultCameraOffset.Magnitude();
            actual_cam_quat = transform.RotationQuat;
        }
        public override void OnUpdate(float dt)
        {
            if (followTarget != null)
            {
                UpdateFollow(dt);
                return;
            }

            Vector2 mouseDelta = Input.GetMouseDelta();
            //SliceLog.Log(mouseDelta.ToString());
            if (!LockCamera)
            {
                float yawDelta = mouseDelta.x * sensitivity.x * dt;
                float pitchDelta = mouseDelta.y * sensitivity.y * dt;

                //apply yaw rotation
                Quaternion yaw_quat = Quaternion.FromAxisAngle(Vector3.Up, yawDelta);
                actual_cam_quat = (yaw_quat * actual_cam_quat).Normalize();
                //apply pitch rotation
                float clampedPitch = Utilities.Clamp(pitch + pitchDelta, yClamp.x, yClamp.y);
                Quaternion pitch_quat = Quaternion.FromAxisAngle(Vector3.Right, clampedPitch - pitch);
                actual_cam_quat = (actual_cam_quat * pitch_quat).Normalize();
                pitch = clampedPitch;
                //apply camera shake
                //set camera's rotation
                transform.RotationQuat = actual_cam_quat * Quaternion.FromEuler(shakeOffset);
                //  Console.Write("helloooo");


                //Hafiz: Idk why Bootstrap.Player was null and crashing when I merged into working
                //       So I did this null check(27/12/2025)
                if (Bootstrap.Player != null)
                {
                    Vector3 basePosition = Bootstrap.Player.transform.Position;

                    Vector3 screenShake = transform.RotationQuat * Vector3.Zero;

                    transform.Position = basePosition + screenShake;
                }
                //float deltaToApply = newPitch - pitch;
                //pitch = newPitch;


            }
            else
            {
                if (shakeOffset != Vector3.Zero)
                {
                    transform.Position += shakeOffset;
                }
            }

            Vector3 dir = cameraChild.GetComponent<Transform>().WorldPosition - this.transform.WorldPosition;



            //SliceLog.Log("Cam dir is " + dir);
            float safeDist = 0f;

            if (Physics.SphereCast(transform.WorldPosition + new Vector3(0, collisionCheckOffset, 0), collisionRadius, dir.Normalize() * defaultCameraOffsetDist, out RayCastHit hit, LayerMask.ToMask("Environment"), QueryTriggerInteraction.Ignore))
            {

                // Place camera just before the surface using the sphere radius
                safeDist = Utilities.Clamp<float>(Math.Max(hit.distance - collisionRadius, 0f), .2f, defaultCameraOffsetDist);
                //SliceLog.Log("safe dist is " + safeDist);
                cameraChild.GetComponent<Transform>().Position = defaultCameraOffset.Normalize() * safeDist;

            }
            else
            {
                cameraChild.GetComponent<Transform>().Position = defaultCameraOffset;
                //SliceLog.Log("Camera aint hitting shit");
            }


            //Vector2 mousePos = Input.GetMousePosition();
            //Console.WriteLine("Mouse Position: X=" + mousePos.x + " Y=" + mousePos.y);
            //if (Input.IsKeyDown(Keys.KEY_J)) transform.Rotate(xSensitivity * dt, Vector3.Up, true);
            //else if (Input.IsKeyDown(Keys.KEY_L)) transform.Rotate(-xSensitivity * dt, Vector3.Up, true);

            //// Vertical rotation (pitch) around local X
            //float pitchDelta = 0f;
            //if (Input.IsKeyDown(Keys.KEY_I)) pitchDelta = ySensitivity * dt;
            //else if (Input.IsKeyDown(Keys.KEY_K)) pitchDelta = -ySensitivity * dt;

            //float newPitch = Utilities.Clamp(pitch + pitchDelta, -60f, 90f);
            //float deltaToApply = newPitch - pitch;
            //pitch = newPitch;

            //transform.Rotate(deltaToApply, Vector3.Forward);
        }
        public Vector3 GetFlatAimDirection(Transform from)
        {
            Vector3 fwd = transform.Forward;
            fwd.y = 0f;
            return fwd.SquareMagnitude() < 1e-4f ? from.Forward : fwd.Normalize();
        }

        public void InitiateCameraMovement()
        {
            //Lock the player movement and the mouse movement so that the player doesn't move during this sequence
            //If the waypoints in the cameraWay make up a straight line, have the camera move in a rectangle movement
            //If the waypoints in the cameraWay dont make up a straight line, have the camera move in a triangle movement where after the camera reaches the last way point it moves back to the original position it was at

            LockCamera = true;

            if (Bootstrap.Player != null)
            {
                Bootstrap.Player.SetPlayerLock(true);
            }

            originalCameraPosition = transform.Position;


            Vector3[] points = new Vector3[cameraWayPoints.Count + 1];

            for (int i = 0; i < cameraWayPoints.Count; i++)
            {
                Transform cameraWaypointTrans = cameraWayPoints[i].GetComponent<Transform>();

                points[i] = cameraWaypointTrans.Position;
            }

            points[cameraWayPoints.Count] = originalCameraPosition;


            if (Utilities.AreAllPointsCollinear3D(points) || (cameraWayPoints.Count < 3))
            {
                //Start Rectangle Movement
                Vector3[] rectPath = GenerateRectanglePath(points);

                Vector3 centerPoint = Utilities.GetShapeCenter(rectPath, 4);

                StartCoroutine(FollowPathSequence(rectPath, centerPoint));
            }
            else
            {
                //Start Triangle Movement
                Vector3 centerPoint = Utilities.GetShapeCenter(points, cameraWayPoints.Count);

                StartCoroutine(FollowPathSequence(points, centerPoint));
            }

        }

        private Vector3[] GenerateRectanglePath(Vector3[] points)
        {

            int waypointCount = points.Length - 1;

            Vector3 startPoint = points[0];
            Vector3 endPoint = points[waypointCount - 1];


            Vector3 lineDir = (endPoint - startPoint);
            if (lineDir.SquareMagnitude() < 0.0001f) return points;
            lineDir = lineDir.Normalize();


            Vector3 rightDir = Vector3.Cross(Vector3.Up, lineDir).Normalize();

            // Failsafe: if the line was perfectly vertical, cross product with Up returns 0
            if (rightDir.SquareMagnitude() < 0.0001f)
            {
                rightDir = Vector3.Cross(Vector3.Forward, lineDir).Normalize();
            }


            float halfWidth = rectangleWidth / 2f;
            Vector3 corner1 = startPoint + (rightDir * halfWidth);
            Vector3 corner2 = endPoint + (rightDir * halfWidth);
            Vector3 corner3 = endPoint - (rightDir * halfWidth);
            Vector3 corner4 = startPoint - (rightDir * halfWidth);


            return new Vector3[] { corner1, corner2, corner3, corner4, points[points.Length - 1] };
        }

        private IEnumerator FollowPathSequence(Vector3[] pathPoints, Vector3 focusPoint)
        {
            for (int i = 0; i < pathPoints.Length; i++)
            {
                Vector3 targetPos = pathPoints[i];

                while (Utilities.Distance3DSquared(transform.Position, targetPos) > 0.01f)
                {
                    // 1. Move the camera
                    transform.Position = Utilities.Lerp(transform.Position, targetPos, cameraTransitionSpeed * Time.deltaTime);

                    // 2. Rotate the camera to face the focus point
                    Vector3 lookDir = (focusPoint - transform.Position);
                    lookDir.y = 0f; // Optional: Keep the camera level. Remove this if you want it to tilt up/down!

                    if (lookDir.SquareMagnitude() > 0.001f) // Prevent errors if we are exactly on the center
                    {
                        lookDir = lookDir.Normalize();
                        Quaternion targetRotation = Quaternion.LookRotation(lookDir, Vector3.Up);

                        // Smoothly rotate towards the target using Slerp
                        transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRotation, cameraRotationSpeed * Time.deltaTime);
                    }

                    yield return null;
                }

                transform.Position = targetPos;
            }

            FinishCameraMovement();
        }

        public Coroutine Shake(float duration, float magnitude)
        {
            return StartCoroutine(ShakeSequence(duration, magnitude));
        }

        private IEnumerator ShakeSequence(float duration, float magnitude)
        {
            float elapsed = 0f;

            while (elapsed < duration)
            {
                //ideally use perlin noise but this shld be fine for now
                float x = SliceRandom.RangeFloat(-1.0f, 1.0f) * magnitude;
                float y = SliceRandom.RangeFloat(-1.0f, 1.0f) * magnitude;
                float z = SliceRandom.RangeFloat(-1.0f, 1.0f) * magnitude;

                shakeOffset = new Vector3(x, y, z);

                elapsed += Time.deltaTime;
                yield return null;
            }

            // Reset offset when finished
            shakeOffset = Vector3.Zero;
        }


        private void FinishCameraMovement()
        {
            LockCamera = false;

            if (Bootstrap.Player != null)
            {
                Bootstrap.Player.SetPlayerLock(false);
            }
            SliceLog.Log("Camera sequence complete. Control returned to player.");
        }

        public void SetFollowTarget(GameObject target)
        {
            followTarget = target;
            LockCamera = true;
        }

        private void UpdateFollow(float dt)
        {
            if (followTarget == null) return;

            Vector3 targetPos = followTarget.GetComponent<Transform>().WorldPosition + followOffset;
            transform.Position = Utilities.Lerp(transform.Position, targetPos, followPositionSmoothness * dt);

            Vector3 lookDir = (followTarget.GetComponent<Transform>().WorldPosition - transform.WorldPosition);
            if (lookDir.SquareMagnitude() > 0.001f)
            {
                lookDir = lookDir.Normalize();
                Quaternion targetRotation = Quaternion.LookRotation(lookDir, Vector3.Up);
                transform.RotationQuat = Quaternion.Slerp(transform.RotationQuat, targetRotation, followRotationSmoothness * dt);
            }
        }
    }
}
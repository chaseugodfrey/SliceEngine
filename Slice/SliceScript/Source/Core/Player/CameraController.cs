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
        //private Vector2 lastMousePos;

        public override void OnCreate()
        {
            //SliceLog.Log("Rotation: x<" + transform.Rotation.x + ">y<" + transform.Rotation.y + ">z<" + transform.Rotation.z);
        }

        public void Initialize()
        {
            //Console.WriteLine("CameraCont Ini called");
        }
        public override void OnUpdate(float dt)
        {
            Vector2 mouseDelta = Input.GetMouseDelta();
            //SliceLog.Log(mouseDelta.ToString());
            if (!LockCamera)
            {
                float yawDelta = mouseDelta.x * sensitivity.x * dt;
                float pitchDelta = mouseDelta.y * sensitivity.y * dt;
                transform.Rotate(yawDelta, Vector3.Up, true);

                float clampedPitch = Utilities.Clamp(pitch + pitchDelta, yClamp.x, yClamp.y);

                //Hafiz: Idk why Bootstrap.Player was null and crashing when I merged into working
                //       So I did this null check(27/12/2025)
                if (Bootstrap.Player != null)
                {
                    transform.Position = Bootstrap.Player.transform.Position;
                }
                //float deltaToApply = newPitch - pitch;
                //pitch = newPitch;

                transform.Rotate(clampedPitch, Vector3.Right);

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

            if(Bootstrap.Player != null)
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
            // The last point in the array is originalCameraPosition. 
            // The waypoints making up the line are everything before it.
            int waypointCount = points.Length - 1;

            Vector3 startPoint = points[0];
            Vector3 endPoint = points[waypointCount - 1];

            // 1. Get the forward direction of the line
            Vector3 lineDir = (endPoint - startPoint);
            if (lineDir.SquareMagnitude() < 0.0001f) return points; // Failsafe if points are identical
            lineDir = lineDir.Normalize();

            // 2. Find the perpendicular direction (Cross Product with Up)
            Vector3 rightDir = Vector3.Cross(Vector3.Up, lineDir).Normalize();

            // Failsafe: if the line was perfectly vertical, cross product with Up returns 0
            if (rightDir.SquareMagnitude() < 0.0001f)
            {
                rightDir = Vector3.Cross(Vector3.Forward, lineDir).Normalize();
            }

            // 3. Calculate the 4 corners
            float halfWidth = rectangleWidth / 2f;
            Vector3 corner1 = startPoint + (rightDir * halfWidth);
            Vector3 corner2 = endPoint + (rightDir * halfWidth);
            Vector3 corner3 = endPoint - (rightDir * halfWidth);
            Vector3 corner4 = startPoint - (rightDir * halfWidth);

            // 4. Return the 4 corners + the original camera position at the end
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

        private void FinishCameraMovement()
        {
            LockCamera = false;

            if (Bootstrap.Player != null)
            {
                PlayerController playerController = Bootstrap.Player.GetComponent<PlayerController>();
                if (playerController != null)
                {
                    playerController.SetPlayerLock(false); // Give control back
                }
            }
            SliceLog.Log("Camera sequence complete. Control returned to player.");
        }
    }
}
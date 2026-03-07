using System;
using System.Collections.Generic;
using System.IO;
using SliceEngine;


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

            if(cameraWayPoints.Count < 3)
            {
                //Start Rectangle Movement
            }

            Vector3[] points = new Vector3[cameraWayPoints.Count + 1];

            for (int i = 0; i < cameraWayPoints.Count; i++)
            {
                Transform cameraWaypointTrans = cameraWayPoints[i].GetComponent<Transform>();

                points[i] = cameraWaypointTrans.Position;
            }

            points[cameraWayPoints.Count] = originalCameraPosition;

            if (Utilities.AreAllPointsCollinear3D(points))
            {
                //Start Rectangle Movement
            }
            else
            {
                //Start Triangle Movement
            }

        }
    }
}
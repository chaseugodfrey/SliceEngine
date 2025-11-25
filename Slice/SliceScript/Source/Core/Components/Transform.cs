using System;

namespace SliceEngine
{
    public class Transform : Component
    {

        // Required for GetComponent<T>() where T : new()
        public Transform() { }
        public Transform(GameObject entity)
        {
            gameObject = entity;
            // Initialize rotationQuat from the current rotation
            FunctionCalls.Transform_GetPosition(gameObject.mID, out Vector3 pos);
            FunctionCalls.Transform_GetScale(gameObject.mID, out Vector3 scale);
            FunctionCalls.Transform_GetRotation(gameObject.mID, out Vector3 euler);
            Position = pos;
            Scale = scale;
            rotationQuat = Quaternion.FromEuler(euler);
        }
        public Vector3 Position
        {
            get
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");
                FunctionCalls.Transform_GetPosition(gameObject.mID, out Vector3 position);
                return position;
            }
            set
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");
                
                FunctionCalls.Transform_SetPosition(gameObject.mID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                FunctionCalls.Transform_GetScale(gameObject.mID, out Vector3 scale);
                return scale;
            }
            set
            {
                FunctionCalls.Transform_SetScale(gameObject.mID, ref value);
            }
        }


        private Quaternion rotationQuat;

        public Vector3 Rotation
        {
            get
            {
                return rotationQuat.ToEuler();
            }
            set
            {                
                FunctionCalls.Transform_SetRotation(gameObject.mID, ref value);
                rotationQuat = Quaternion.FromEuler(value);
            }
        }

        public Quaternion RotationQuat
        {
            get => rotationQuat;
            set
            {
                rotationQuat = value;
                Vector3 euler = rotationQuat.ToEuler();
                FunctionCalls.Transform_SetRotation(gameObject.mID, ref euler);
            }
        }
        public Vector3 Right { get { return RotationQuat * Vector3.Right; } }
        public Vector3 Up { get { return RotationQuat * Vector3.Up; } }
        public Vector3 Forward { get { return RotationQuat * Vector3.Forward; } }
        public void Translate(Vector3 translation, bool localSpace = false)
        {
            Vector3 pos = Position;

            if (localSpace)
            {
                // Convert local translation to world space
                Vector3 rotatedTranslation = RotationQuat * translation;
                pos += rotatedTranslation;
            }
            else
            {
                pos += translation;
            }

            Position = pos;
        }

        public void Rotate(float angleDegrees, Vector3 axis)
        {       
            if (axis == Vector3.Zero)
                return; // No rotation if axis is zero

            Quaternion delta = Quaternion.FromAxisAngle(axis.Normalize(), angleDegrees);

            Quaternion tempRotationQuat = RotationQuat * delta;

            tempRotationQuat.Normalize();

            Vector3 rotation = tempRotationQuat.ToEuler();
            rotation.x = 0.0f;

            //if (gameObject.HasComponent<CameraController>())
            {
                Vector3 newUp = tempRotationQuat * Vector3.Up;
                SliceLog.Log("Test New Up" + newUp.x + "," + newUp.y + "," + newUp.z);
                //if(Vector3.Dot(newUp, originalUp) < 0.0f)
                //    return;
            }

            FunctionCalls.Transform_SetRotation(gameObject.mID, ref rotation);

            Rotation = rotation;
        }

        public void LookAt(Vector3 targetPosition)
        {
            Vector3 direction = targetPosition - Position;
            if (direction.Distance(targetPosition) < 1e-6f)
                return; // no rotation if positions are the same

            Rotation = Quaternion.LookRotation(direction, new Vector3(0, 1, 0)).ToEuler();
        }
        public void LookAt(Vector3 targetPosition, Vector3 up)
        {
            Vector3 direction = targetPosition - Position;
            if (direction.LengthSquared() < 1e-6f)
                return;

            Rotation = Quaternion.LookRotation(direction, up).ToEuler();
        }
        public bool IsFlipped()
        {
            Vector3 originalUp = new Vector3(0, 1, 0);
            Vector3 newUp = RotationQuat * originalUp;
            return Vector3.Dot(newUp, originalUp) > 0.0f;
        }
    }
}
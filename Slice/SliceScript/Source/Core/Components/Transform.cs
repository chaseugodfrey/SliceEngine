using System;
using static SliceEngine.Cursor;

namespace SliceEngine
{
    public class Transform : Component
    {
        private Vector3 _position = Vector3.Zero;
        private Vector3 _scale = Vector3.One;
        private Quaternion _rotationQuat = Quaternion.Identity;

        // Required for GetComponent<T>() where T : new()
        public Transform() { }
        public Transform(GameObject entity)
        {
            gameObject = entity;
            // Initialize rotationQuat from the current rotation
            FunctionCalls.Transform_GetPosition(gameObject.mID, out Vector3 _position);
            FunctionCalls.Transform_GetScale(gameObject.mID, out Vector3 _scale);
            FunctionCalls.Transform_GetRotationQuat(gameObject.mID, out Quaternion _rotQuat);

            Position = _position;
            Scale = _scale;
            RotationQuat = _rotQuat;
        }
        public Vector3 Position
        {
            get
            {
                FunctionCalls.Transform_GetPosition(gameObject.mID, out Vector3 _position);

                return _position;
            }
            set
            {
                _position = value;
                FunctionCalls.Transform_SetPosition(gameObject.mID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                FunctionCalls.Transform_GetScale(gameObject.mID, out Vector3 _scale);
                return _scale;
            }
            set
            {
                _scale = value;
                FunctionCalls.Transform_SetScale(gameObject.mID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                FunctionCalls.Transform_GetRotationQuat(gameObject.mID, out Quaternion _rotQuat);

                return _rotQuat.ToEuler();
            }
            set
            {
                _rotationQuat = Quaternion.FromEuler(value).Normalize();
                FunctionCalls.Transform_SetRotationQuat(gameObject.mID, ref _rotationQuat);
            }
        }

        public Quaternion RotationQuat
        {
            get
            {
                FunctionCalls.Transform_GetRotationQuat(gameObject.mID, out Quaternion _rotQuat);

                return _rotQuat;
            }
            set
            {
                _rotationQuat = value.Normalize();
                FunctionCalls.Transform_SetRotationQuat(gameObject.mID, ref _rotationQuat);
            }
        }
        public Vector3 WorldPosition
        {
            get
            {
                FunctionCalls.Transform_GetWorldPosition(gameObject.mID, out Vector3 worldPosition);
                return worldPosition;
            }
        }
        public Quaternion WorldRotationQuat
        {
            get
            {
                FunctionCalls.Transform_GetWorldRotationQuat(gameObject.mID, out Quaternion worldRotataionQuat);
                return worldRotataionQuat;
            }
        }

        public Vector3 Right    => RotationQuat * Vector3.Right;
        public Vector3 Left     => RotationQuat * Vector3.Left;
        public Vector3 Up       => RotationQuat * Vector3.Up;
        public Vector3 Down     => RotationQuat * Vector3.Down;
        public Vector3 Forward  => RotationQuat * Vector3.Forward;
        public Vector3 Backward => RotationQuat * Vector3.Backward;
        public void Pitch(float degrees) => RotateLocalAxis(Vector3.Right, degrees);
        public void Yaw(float degrees) => RotateLocalAxis(Vector3.Up, degrees);
        public void Roll(float degrees) => RotateLocalAxis(Vector3.Forward, degrees);
        public void Translate(Vector3 translation, bool localSpace = false)
        {
            if (localSpace)
            {
                _position += _rotationQuat * translation;
            }
            else
            {
                _position += translation;
            }

            FunctionCalls.Transform_SetPosition(gameObject.mID, ref _position);
        }

        public void Rotate(float angleDegrees, Vector3 axis, bool preRotate = false)
        {
            if (axis == Vector3.Zero) return;

            Quaternion delta = Quaternion.FromAxisAngle(axis.Normalize(), angleDegrees);
            if(preRotate)
            {
                RotationQuat = (delta * RotationQuat).Normalize();
            }
            else
            {
                RotationQuat = (RotationQuat * delta).Normalize();
            }
        }

        public void Rotate(Vector3 eulerDegrees)
        {
            Quaternion delta = Quaternion.FromEuler(eulerDegrees).Normalize();
            RotationQuat = (RotationQuat * delta).Normalize();
        }

        public void Rotate(Vector3 eulerDegrees, bool preRotate = false)
        {
            Quaternion delta = Quaternion.FromEuler(eulerDegrees).Normalize();

            _rotationQuat = preRotate
                ? (delta * _rotationQuat).Normalize()   
                : (_rotationQuat * delta).Normalize();

            FunctionCalls.Transform_SetRotationQuat(gameObject.mID, ref _rotationQuat);
        }

        public void RotateAxisAngle(Vector3 axis, float angleDegrees)
        {
            if (axis.LengthSquared() < 1e-6f) return;

            Quaternion delta = Quaternion.FromAxisAngle(axis.Normalize(), angleDegrees);
            RotationQuat = (RotationQuat * delta).Normalize();
        }

        public void RotateAround(Vector3 point, Vector3 axis, float angleDegrees)
        {
            if (axis.LengthSquared() < 1e-6f) return;

            // Build rotation
            Quaternion delta = Quaternion.FromAxisAngle(axis.Normalize(), angleDegrees);

            // Move position
            Vector3 dir = Position - point;
            dir = delta * dir;        // rotate direction vector
            Position = point + dir;   // apply

            // Rotate orientation
            RotationQuat = (delta * RotationQuat).Normalize();
        }

        public void RotateTowards(Quaternion target, float maxDegrees)
        {
            // Compute shortest rotation
            Quaternion delta = Quaternion.RotateTowards(RotationQuat, target, maxDegrees);
            RotationQuat = delta.Normalize();
        }

        public void RotateLocalAxis(Vector3 localAxis, float angleDegrees)
        {
            // Convert to world axis
            Vector3 worldAxis = RotationQuat * localAxis;

            Quaternion delta = Quaternion.FromAxisAngle(worldAxis.Normalize(), angleDegrees);
            RotationQuat = (delta * RotationQuat).Normalize();
        }

        public void LookAt(Vector3 targetPosition, Vector3 up)
        {
            Vector3 direction = targetPosition - Position;

            // Avoid zero-length direction
            if (direction.LengthSquared() < 1e-6f)
                return;

            // Ensure 'up' is valid
            if (up.LengthSquared() < 1e-6f)
                up = Vector3.Up;

            // Build rotation quaternion
            Quaternion lookRot = Quaternion.LookRotation(direction.Normalize(), up.Normalize());

            RotationQuat = lookRot.Normalize();
        }

        public bool IsFlipped()
        {
            Vector3 originalUp = new Vector3(0, 1, 0);
            Vector3 newUp = RotationQuat * originalUp;
            return Vector3.Dot(newUp, originalUp) > 0.0f;
        }
    }
}
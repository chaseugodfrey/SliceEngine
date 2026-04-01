using System;

namespace SliceEngine
{

    public class ColliderShape : Component
    {
        public ColliderShape() { }
        public ColliderShape(GameObject entity)
        {
            gameObject = entity;
            ComponentEnabled = FunctionCalls.ColliderShape_IsEnabled(gameObject.mID);
        }

        public bool ComponentEnabled
        {
            get
            {
                return FunctionCalls.ColliderShape_IsEnabled(gameObject.mID);
            }
            set
            {
                FunctionCalls.ColliderShape_SetEnabled(gameObject.mID, value);
            }
        }

    }
}
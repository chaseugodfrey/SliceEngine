using System;

namespace SliceEngine
{

    public class ColliderShape : Component
    {
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
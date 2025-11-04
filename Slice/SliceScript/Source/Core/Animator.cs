using System;

namespace SliceEngine
{
    public class Animator : Component
    {
        public void ChangeAnim(uint animIndex)
        {
            FunctionCalls.ChangeAnim(Entity.mID, animIndex);
        }
    }
}
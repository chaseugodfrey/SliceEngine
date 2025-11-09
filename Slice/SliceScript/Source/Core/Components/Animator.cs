using System;

namespace SliceEngine
{
    public class Animator : Component
    {
        public Animator() { }
        public Animator(GameObject entity)
        {
            gameObject = entity;
        }
        public void ChangeAnim(uint animIndex)
        {
            FunctionCalls.ChangeAnim(gameObject.mID, animIndex);
        }

        public void SetBool(string name, bool value)
        {
            FunctionCalls.SetBool(gameObject.mID, name, value);
        }

        public void SetFloat(string name, float value)
        {
            FunctionCalls.SetFloat(gameObject.mID, name, value);
        }

        public void SetInt(string name, int value)
        {
            FunctionCalls.SetInt(gameObject.mID, name, value);
        }
    }
}
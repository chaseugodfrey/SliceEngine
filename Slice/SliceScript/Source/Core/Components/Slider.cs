
namespace SliceEngine
{
    public class Slider : Component
    {
        public Slider() { }
        public Slider(GameObject entity)
        {
            gameObject = entity;
        }

        public void SetValue(float value)
        {
            FunctionCalls.Slider_SetValue(gameObject.mID, value);
        }

        public float GetValue()
        {
            return FunctionCalls.Slider_GetValue(gameObject.mID);
        }
    }
}
namespace SliceEngine
{
    public class RectTransform : Component
    {
        public RectTransform() { }
        public RectTransform(GameObject entity)
        {
            gameObject = entity;
        }

        public enum HoriPivot
        {
            LEFT,
            CENTER,
            RIGHT,
            STRETCH_H
        };
        public enum VertPivot
        {
            TOP,
            MIDDLE,
            BOTTOM,
            STRETCH_V
        };
        public HoriPivot Horizontal
        {
            get
            {
                FunctionCalls.RectTransform_GetHoriAlign(gameObject.mID, out HoriPivot value);
                return value;
            }
            set
            {
                FunctionCalls.RectTransform_SetHoriAlign(gameObject.mID, ref value);
            }
        }
        public VertPivot Vertical
        {
            get
            {
                FunctionCalls.RectTransform_GetVertAlign(gameObject.mID, out VertPivot value);
                return value;
            }
            set
            {
                FunctionCalls.RectTransform_SetVertAlign(gameObject.mID, ref value);
            }
        }

    }
}
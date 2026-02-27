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

        public int Pos_X
        {
            get
            {
                return FunctionCalls.RectTransform_GetPosX(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetPosX(gameObject.mID, value);
            }
        }
        public int Pos_Y
        {
            get
            {
                return FunctionCalls.RectTransform_GetPosY(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetPosY(gameObject.mID, value);
            }
        }
        public int Width
        {
            get
            {
                return FunctionCalls.RectTransform_GetWidth(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetWidth(gameObject.mID, value);
            }
        }
        public int Height
        {
            get
            {
                return FunctionCalls.RectTransform_GetHeight(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetHeight(gameObject.mID, value);
            }
        }

        public int Top
        {
            get
            {
                return FunctionCalls.RectTransform_GetTop(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetTop(gameObject.mID, value);
            }
        }
        public int Bot
        {
            get
            {
                return FunctionCalls.RectTransform_GetBot(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetBot(gameObject.mID, value);
            }
        }
        public int Left
        {
            get
            {
                return FunctionCalls.RectTransform_GetLeft(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetLeft(gameObject.mID, value);
            }
        }
        public int Right
        {
            get
            {
                return FunctionCalls.RectTransform_GetRight(gameObject.mID);
            }
            set
            {
                FunctionCalls.RectTransform_SetRight(gameObject.mID, value);
            }
        }
    }
}
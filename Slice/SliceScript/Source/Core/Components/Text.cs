
namespace SliceEngine
{
    public class FontRenderer : Component
    {
        public FontRenderer() { }
        public FontRenderer(GameObject entity)
        {
            gameObject = entity;
        }

        //Font size in pixels relative to canvas
        public float Fontsize
        {
            get
            {
                return FunctionCalls.FontRenderer_GetFontsize(gameObject.mID);
            }
            set
            {
                FunctionCalls.FontRenderer_SetFontsize(gameObject.mID, value);
            }
        }

        public float Linespacing
        {
            get
            {
                return FunctionCalls.FontRenderer_GetLinespacing(gameObject.mID);
            }
            set
            {
                FunctionCalls.FontRenderer_SetLinespacing(gameObject.mID, value);
            }
        }

        public Vector4 Colour
        {
            get
            {
                FunctionCalls.FontRenderer_GetColor(gameObject.mID, out Vector4 value);
                return value;
            }
            set
            {
                FunctionCalls.FontRenderer_SetColor(gameObject.mID, ref value);
            }
        }

        public string Text_val
        {
            get 
            {
                return FunctionCalls.FontRenderer_GetText(gameObject.mID);
            }
            set
            {
                FunctionCalls.FontRenderer_SetText(gameObject.mID, value);
            } 
        }


        public void SetEnabled(bool enabled)
        {
            FunctionCalls.FontRenderer_SetEnabled(gameObject.mID, enabled);
        }
    }
}
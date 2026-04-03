
namespace SliceEngine
{
    public class SpriteAnimator : Component
    {
        public SpriteAnimator() { }
        public SpriteAnimator(GameObject entity)
        {
            gameObject = entity;
        }

        public bool IsPlaying
        {
            get
            {
                return FunctionCalls.SpriteAnimator_GetPlaying(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteAnimator_SetPlaying(gameObject.mID, value);
            }
        }
        public bool Loop
        {
            get
            {
                return FunctionCalls.SpriteAnimator_GetLoop(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteAnimator_SetLoop(gameObject.mID, value);
            }
        }
        public uint Rows
        {
            get
            {
                return FunctionCalls.SpriteAnimator_GetRows(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteAnimator_SetRows(gameObject.mID, value);
            }
        }
        public uint Columns
        {
            get
            {
                return FunctionCalls.SpriteAnimator_GetCols(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteAnimator_SetCols(gameObject.mID, value);
            }
        }

        public uint FrameCount
        {
            get
            {
                return FunctionCalls.SpriteAnimator_GetFrameCnt(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteAnimator_SetFrameCnt(gameObject.mID, value);
            }
        }
        public float FPS
        {
            get
            {
                return FunctionCalls.SpriteAnimator_GetFPS(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteAnimator_SetFPS(gameObject.mID, value);
            }
        }

        public uint CurrentFrame
        {
            get
            {
                return FunctionCalls.SpriteAnimator_GetCurrFrame(gameObject.mID);
            }
            set
            {
                FunctionCalls.SpriteAnimator_SetCurrFrame(gameObject.mID, value);
            }
        }

        public void SetEnabled(bool enabled)
        {
            FunctionCalls.SpriteAnimator_SetEnabled(gameObject.mID, enabled);
        }
    }
}
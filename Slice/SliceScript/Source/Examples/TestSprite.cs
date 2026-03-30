using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestSprite : SliceBehaviour
    {
        SpriteAnimator sprite;
        //RectTransform rect;
        public override void OnCreate()
        {
            sprite = GetComponent<SpriteAnimator>();
            SliceLog.Log("Creating sprite anim test script");
            sprite.Rows = 2;
            sprite.Columns = 2;
            sprite.FrameCount = 4;
            sprite.FPS = 2f;
            sprite.Loop = true;
        }

        public override void OnUpdate(float dt)
        {
            if(Input.IsKeyPressed(Keys.KEY_H))
            {
                sprite.Rows = 2;
                sprite.Columns = 2;
                sprite.FrameCount = 4;
                sprite.CurrentFrame = 1;
                sprite.FPS = 0.5f;
                sprite.Loop = false;
            }
            if (Input.IsKeyPressed(Keys.KEY_J))
            {
                sprite.Rows = 5;
                sprite.Columns = 5;
                sprite.FrameCount = 25;
                sprite.Loop = true;
                sprite.FPS = 5f;
                sprite.CurrentFrame = 5;
                sprite.IsPlaying = true;
            }
            if (Input.IsKeyPressed(Keys.KEY_A))
            {
                sprite.IsPlaying = true;
            }
            if (Input.IsKeyPressed(Keys.KEY_S))
            {
                sprite.IsPlaying = false;
            }
        }
        public TestSprite() { }
    };

}
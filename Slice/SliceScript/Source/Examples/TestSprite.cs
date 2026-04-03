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
            //sprite.CurrentFrame = 5;
            //sprite.IsPlaying = false;
            //sprite.Loop = false;
        }

        public override void OnUpdate(float dt)
        {
        }
        public TestSprite() { }
    };

}
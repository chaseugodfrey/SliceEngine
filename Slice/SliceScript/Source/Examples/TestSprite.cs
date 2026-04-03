using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestSprite : SliceBehaviour
    {
        SpriteRenderer sprite;
        //RectTransform rect;
        public override void OnCreate()
        {
            sprite = GetComponent<SpriteRenderer>();
            SliceLog.Log("Creating sprite renderer test script");
            sprite.Colour = new Vector4(0.2f, 0.8f, 0.5f, 0.8f);
        }

        public override void OnUpdate(float dt)
        {
        }
        public TestSprite() { }
    };

}
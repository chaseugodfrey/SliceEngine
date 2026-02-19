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
            SliceLog.Log("Creating sprite test script");

            sprite.Colour = (new Vector4(1.0f, 0.0f, 0.0f, 0.5f));
        }

        public override void OnUpdate(float dt)
        {
            if(Input.IsKeyPressed(Keys.KEY_H))
            {
                sprite.SetEnabled(true);
            }
            if (Input.IsKeyPressed(Keys.KEY_J))
            {
                sprite.SetEnabled(false);
            }
        }
        public TestSprite() { }
    };

}
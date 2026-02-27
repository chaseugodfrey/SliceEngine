using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestRectTransform : SliceBehaviour
    {
        RectTransform rect;
        public override void OnCreate()
        {
            rect = GetComponent<RectTransform>();
            SliceLog.Log("Creating rect test script");

        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_H))
            {
                rect.Horizontal = RectTransform.HoriPivot.LEFT;
                rect.Pos_X = 100;
            }
            if (Input.IsKeyPressed(Keys.KEY_L))
            {
                rect.Horizontal = RectTransform.HoriPivot.RIGHT;
                rect.Pos_X = -200;
            }

            if (Input.IsKeyPressed(Keys.KEY_J))
            {
                rect.Vertical = RectTransform.VertPivot.TOP;
                rect.Pos_Y = -100;
            }
            if (Input.IsKeyPressed(Keys.KEY_K))
            {
                rect.Vertical = RectTransform.VertPivot.BOTTOM;
                rect.Pos_Y = 200;
            }


            if (Input.IsKeyPressed(Keys.KEY_A))
            {
                rect.Horizontal = RectTransform.HoriPivot.CENTER;
                rect.Pos_X = -100;
            }
            if (Input.IsKeyPressed(Keys.KEY_S))
            {
                rect.Vertical = RectTransform.VertPivot.MIDDLE;
                rect.Pos_Y = -100;
            }

            if (Input.IsKeyPressed(Keys.KEY_D))
            {
                rect.Horizontal = RectTransform.HoriPivot.STRETCH_H;
                rect.Left = 50;
                rect.Right = 100;
            }
            if (Input.IsKeyPressed(Keys.KEY_F))
            {
                rect.Vertical = RectTransform.VertPivot.STRETCH_V;
                rect.Top = 100;
                rect.Bot = 50;
            }
        }
        public TestRectTransform() { }
    };

}
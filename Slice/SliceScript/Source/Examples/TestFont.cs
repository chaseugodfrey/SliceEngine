using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestFontrenderer : SliceBehaviour
    {
        FontRenderer text;
        //RectTransform rect;
        public override void OnCreate()
        {
            text = GetComponent<FontRenderer>();
            SliceLog.Log("Creating font test script");

            SliceLog.Log("\nCurrent font details: " +
                "\nFontsize: " + text.Fontsize +
                "\nLinespacing: " + text.Linespacing +
                "\nText: " + text.Text_val +
                "\nColor: " + text.Colour);

            text.Colour = (new Vector4(1.0f, 0.0f, 0.0f, 1.0f));
            text.Fontsize = (60);
            text.Linespacing = (1.2f);
            text.Text_val = ("testing setting text from script :) \nThis can have multiple lines");

            SliceLog.Log("\nModified font details: " +
                "\nFontsize: " + text.Fontsize +
                "\nLinespacing: " + text.Linespacing +
                "\nText: " + text.Text_val +
                "\nColor: " + text.Colour);
        }

        public override void OnUpdate(float dt)
        {
            if(Input.IsKeyPressed(Keys.KEY_H))
            {
                text.SetEnabled(true);
            }
            if (Input.IsKeyPressed(Keys.KEY_J))
            {
                text.SetEnabled(false);
            }
        }
        public TestFontrenderer() { }
    };

}
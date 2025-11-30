using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestButtonSlider : SliceBehaviour
    {
        int test = 0;

        Slider slider;
        public override void OnButtonClick()
        {
            test++;

            SliceLog.Log("Test click"+ test);
        }
        public override void OnButtonRelease()
        {
            test++;

            SliceLog.Log("Test release" + test);
        }

        public override void OnSliderValue(float value)
        {
            SliceLog.Log("Slider value changed waor" + value);
        }

        public override void OnCreate()
        {
            slider = GetComponent<Slider>();
            SliceLog.Log("Creating slider button test script");
        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_B))
            {
                float curr_val = slider.GetValue();
                SliceLog.Log("Current Slider Value: " + curr_val);
                curr_val += 0.1f;
                while (curr_val > 1.0f)
                {
                    curr_val -= 1.0f;
                }

                SliceLog.Log("New Slider Value: " + curr_val);
                slider.SetValue(curr_val);
            }
        }
    }
}

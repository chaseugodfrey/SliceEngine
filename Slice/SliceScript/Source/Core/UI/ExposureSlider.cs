using SliceEngine;
using SliceScript.Source.Core.Systems;

namespace SliceEngine
{
    public class ExposureSlider : SliceBehaviour
    {
        private Slider slider;
        float gammaValue = 0f;

        public override void OnCreate()
        {
            slider = GetComponent<Slider>();
            if (slider != null)
            {
                slider.SetValue(Camera.Exposure * 0.1f);
            }
        }

        public override void OnSliderValue(float value)
        {
            gammaValue = value * 10;
            Camera.Exposure = gammaValue;
        }
    }
}

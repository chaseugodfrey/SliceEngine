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
                slider.SetValue(Camera.Gamma * 0.1f);
            }
        }

        public override void OnUpdate(float dt)
        {
            
        }

        public void RestoreDefaultGamma()
        {
            Camera.Gamma = 45.5f;
            
        }

        public override void OnSliderValue(float value)
        {
            if(Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_LEFT))
            {
                AudioSettings.PlaySFX("UIClick");
            }

            gammaValue = value * 10;
            Camera.Gamma = gammaValue;
        }
    }
}

using SliceEngine;
using SliceScript.Source.Core.Systems;

namespace SliceEngine
{
    public class VolumeSlider : SliceBehaviour
    {
        public string audioParameter = "Master";

        private Slider mySlider;

        public override void OnCreate()
        {
            mySlider = GetComponent<Slider>();

            if (mySlider != null)
            {
               
                if (audioParameter == "Master")
                {

                    mySlider.SetValue(AudioManager.GetMasterVolume());
                }
                else
                {

                    mySlider.SetValue(AudioManager.GetCategoryVolume(audioParameter));
                }
            }
        }

        

        public override void OnSliderValue(float value)
        {
            
            if (audioParameter == "Master")
            {
                AudioManager.SetMasterVolume(value);
            }
            else
            {
                SliceLog.Log(audioParameter);
                AudioManager.SetCategoryVolume(audioParameter, value);
            }
        }
    }
}
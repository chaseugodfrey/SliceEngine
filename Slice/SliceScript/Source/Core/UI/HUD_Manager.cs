using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class HUD_Manager: SliceBehaviour, IInitializable
    {

        Slider health;
        string healthSliderName;

        

        public override void OnCreate()
        {

        }


        public void SetHealth(float input)
        {
            health.SetValue(input);
        }

        public override void OnButtonClick()
        {
            SliceLog.Log("Button CLick");
        }

        public override void OnButtonRelease()
        {
            SliceLog.Log("Button Release");
        }

        public void Initialize()
        {
            health = gameObject.FindGameObjectWithName(healthSliderName).GetComponent<Slider>();
        }
    }
}

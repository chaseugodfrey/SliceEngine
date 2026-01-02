using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class HUD_Manager: SliceBehaviour, IInitializable
    {

        Slider health;
        public string healthSliderName;

        SpriteRenderer victory;
        public string victoryName;
        SpriteRenderer defeat;
        public string defeatName;
        public override void OnCreate()
        {

        }


        public void SetHealth(float input)
        {
            health.SetValue(input);
        }

        public void GameWinScreen()
        {
            victory.SetEnabled(true);
        }

        public void GameLoseScreen()
        {

            defeat.SetEnabled(true);
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
            Console.WriteLine("HUD Ini called");
            health = gameObject.FindGameObjectWithName(healthSliderName).GetComponent<Slider>();
            victory = gameObject.FindGameObjectWithName(victoryName).GetComponent<SpriteRenderer>();
            defeat = gameObject.FindGameObjectWithName(defeatName).GetComponent<SpriteRenderer>();
        }
    }
}

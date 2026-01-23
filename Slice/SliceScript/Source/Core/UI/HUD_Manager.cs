using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class HUD_Manager: SliceBehaviour, IInitializable
    {

        Slider health;
        public GameObject healthSliderName;

        SpriteRenderer victory;
        public GameObject victoryName;

        SpriteRenderer defeat;
        public GameObject defeatName;
        public override void OnCreate()
        {

        }


        public void SetHealth(float input)
        {
            Console.WriteLine("Setting health");
            health.SetValue(input);
            Console.WriteLine("Finish setting health");
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
            health = healthSliderName.GetComponent<Slider>();
            victory = victoryName.GetComponent<SpriteRenderer>();
            defeat = defeatName.GetComponent<SpriteRenderer>();
        }
    }
}

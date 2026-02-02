using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class HUD_Manager: SliceBehaviour, IInitializable
    {

        Slider health;
        public GameObject healthSliderObject;

        SpriteRenderer victory;
        public GameObject victoryObject;

        SpriteRenderer defeat;
        public GameObject defeatObject;

        public GameObject textBoxObject;
        public override void OnCreate()
        {

        }


        public void SetHealth(float input)
        {
            Console.WriteLine("Setting health");
            //health.SetValue(input);
            healthSliderObject.GetComponent<Slider>().SetValue(input);
            Console.WriteLine("Finish setting health");
        }

        public void GameWinScreen()
        {
            //victory.SetEnabled(true);
            victoryObject.GetComponent<SpriteRenderer>().SetEnabled(true);
        }

        public void GameLoseScreen()
        {
            //defeat.SetEnabled(true);
            defeatObject.GetComponent<SpriteRenderer>().SetEnabled(true);
        }

        public override void OnButtonClick()
        {
            SliceLog.Log("Button CLick");
        }

        public override void OnButtonRelease()
        {
            SliceLog.Log("Button Release");
        }

        public void LoadDialogues()
        {
            //Load dialogues from a CSV
        }

        public void SetTextBox(string input)
        {
            textBoxObject.SetActive(true);

            //Set Text
        }

        public void CloseTextBox()
        {
            textBoxObject.SetActive(false);
        }

        public void Initialize()
        {
            Console.WriteLine("HUD Ini called");
            health = healthSliderObject.GetComponent<Slider>();      
            victory = victoryObject.GetComponent<SpriteRenderer>();  
            defeat = defeatObject.GetComponent<SpriteRenderer>();    
        }
    }
}

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

        public GameObject textBoxParentObject;
        public GameObject regularTextObject;

        //Letters per second
        public float typeSpeed = 5f;

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (Input.IsKeyDown(Keys.KEY_M))
            {
                PlayDialogueForLevel(0);
            }

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

        private SliceCSV loader = new SliceCSV();
        public void LoadDialogues()
        {
            //Load dialogues from a CSV

            loader.Load("C:\\Users\\User\\Desktop\\Y3Tri1\\GAM 300\\SliceEngine\\Slice\\SliceEditor\\Assets\\[80]Streaming_Assets\\Dialogue.csv");

            if (loader == null)
            {
                SliceLog.Log("Loader is empty");
            }
            else
            {
                SliceLog.Log("Load has this many rows" + loader.RowCount);
            }

        }

        private bool typing = false;
        
        private List<string> levelDialogues = new List<string>();

        private int dialogueIndex = 0;

        public void PlayDialogueForLevel(int level)
        {

            if (typing == true)
            {
                typing = false;
                return;
            }

            if (levelDialogues.Count > 0)
            {
                // dialogues is not empty
                //  tick up number

                SliceLog.Log("Dialogue is not empty");
                dialogueIndex++;

            }
            else
            {

                SliceLog.Log("Dialogue is empty");
                dialogueIndex = 0;
                for (int i = loader.FindRowIndex( "Level",level.ToString()); i > -1 ; i++)
                {
                    SliceLog.Log("index is at" + i);

                    if (loader.GetValue(i, "Level") != level.ToString())
                    {
                        break;
                    }

                    levelDialogues.Add(loader.GetValue(i, "Text"));
                }
            }


            OpenTextBox();
            typing = true;
            StartCoroutine(TypeText(levelDialogues[dialogueIndex]));

            if (levelDialogues.Count == dialogueIndex + 1)
            {
                // end of dialogue stack
                // clear stack

                dialogueIndex = 0;
                levelDialogues.Clear();
            }
        }

        IEnumerator TypeText(string toType)
        {
            float timecounter = 0f;
            float speed = 1f / typeSpeed;
            string displaying = "";

            SliceLog.Log("To type is:" + toType);
            for (int i = 0; i < toType.Length; i++)
            {

                if (typing == false)
                {
                    break;
                }

                SliceLog.Log("Displaying is " + displaying);
                displaying += toType[i];

                SetTextBox(displaying);

                yield return new WaitForSeconds(speed);
            }

            SetTextBox(toType);

            yield break;
        }



        #region Textbox controls


        public void SetTextBox(string input)
        {
            SliceLog.Log("Setting Textbox");

            if (regularTextObject.HasComponent<FontRenderer>())
            {
                SliceLog.Log("Has Font");
                regularTextObject.GetComponent<FontRenderer>().Text_val = input;
            }
            else
            {
                SliceLog.Log("No Font component");
            }
            //Set Text
        }

        public void  OpenTextBox()
        {
            textBoxParentObject.SetActive(true);
        }

        public void CloseTextBox()
        {
            textBoxParentObject.SetActive(false);
        }

        #endregion

        public void Initialize()
        {
            Console.WriteLine("HUD Ini called");
            health = healthSliderObject.GetComponent<Slider>();      
            victory = victoryObject.GetComponent<SpriteRenderer>();
            defeat = defeatObject.GetComponent<SpriteRenderer>();
            LoadDialogues();
        }
    }
}

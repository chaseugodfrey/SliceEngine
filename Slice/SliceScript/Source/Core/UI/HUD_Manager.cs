using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class HUD_Manager: SliceBehaviour, IInitializable
    {
        public bool DialogueOnStart = false;
        public int currLevel = 0; // to sync with level director

        Slider health;
        public GameObject healthSliderObject;

        SpriteRenderer victory;
        public GameObject victoryObject;

        SpriteRenderer defeat;
        public GameObject defeatObject;

        public GameObject textBoxParentObject;
        public GameObject regularTextObject;
        public GameObject nameTextObject;

        //Letters per second
        public float typeSpeed = 5f;

        private bool enterPressed = false;
        private bool inputOpen = false;
        public bool dialogueDone = false;

        public override void OnCreate()
        {
            if (DialogueOnStart)
            {
                PlayDialogueForLevel(currLevel);
                Bootstrap.Player.canInput = false;
            }
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (inputOpen)
            {
                if (Input.IsKeyDown(Keys.KEY_F) && enterPressed == false)
                {
                    enterPressed = true;
                    PlayDialogueForLevel(currLevel);
                }

                if (Input.IsKeyReleased(Keys.KEY_F) && enterPressed == true)
                {
                    enterPressed = false;
                }
            }

        }


        public void SetHealth(float input)
        {
            //console.writeline("Setting health");
            //health.SetValue(input);
            healthSliderObject.GetComponent<Slider>().SetValue(input);
            //console.writeline("Finish setting health");
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
            //SliceLog.Log("Button CLick");
        }

        public override void OnButtonRelease()
        {
            //SliceLog.Log("Button Release");
        }

        private SliceCSV loader = new SliceCSV();
        public void LoadDialogues()
        {
            //Load dialogues from a CSV
            // SliceLog.Log("Streaming Assets filepath: " + Application.streamingAssetsPath);
            string filePath = Application.GetFilePath("[80]Streaming_Assets/Dialogue.csv");
            SliceLog.Log("App filepath: " + filePath);
            loader.Load(filePath);

            if (loader == null)
            {
                //SliceLog.Log("Loader is empty");
            }
            else
            {
                //SliceLog.Log("Load has this many rows" + loader.RowCount);
            }

        }

        private bool typing = false;
        
        //string[] for listed things 0 = name, 1 = text
        private List<string[]> levelDialogues = new List<string[]>();

        private int dialogueIndex = 0;

        public void PlayDialogueForLevel(int level)
        {


            // Skip to display full line when type writer effect is playing.
            if (typing == true)
            {
                typing = false;
                return;
            }

            //Close dialogue box if it is the last line of the set
            if (levelDialogues.Count == dialogueIndex + 1)
            {
                // end of dialogue stack
                // clear stack

                currLevel++; // increment curr level to prevent reloading same dialogue set
                inputOpen = false;
                dialogueDone = true;
                Bootstrap.Player.canInput = true;
                dialogueIndex = 0;
                levelDialogues.Clear();
                SetTextBox("");
                CloseTextBox();
                return;
            }

            // Will tick dialogue up if it is already loaded, else will load fresh set and play
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
                    //SliceLog.Log("index is at" + i);

                    if (loader.GetValue(i, "Level") != level.ToString())
                    {
                        break;
                    }
                    else if (int.Parse(loader.GetValue(i, "Level")) < currLevel)
                    {
                        break;
                    }

                    dialogueDone = false;

                     levelDialogues.Add(new string[] { loader.GetValue(i, "Name"), loader.GetValue(i, "Text") });
                }
            }


            OpenTextBox();
            typing = true;
            StartCoroutine(TypeText(levelDialogues[dialogueIndex][1]));
            SetName(levelDialogues[dialogueIndex][0]);
            inputOpen = true;
            currLevel = level;
        }

        IEnumerator TypeText(string toType)
        {
            //float timecounter = 0f;
            float speed = 1f / typeSpeed;
            string displaying = "";

            //SliceLog.Log("To type is:" + toType);
            for (int i = 0; i < toType.Length; i++)
            {

                if (typing == false)
                {
                    break;
                }

                //SliceLog.Log("Displaying is " + displaying);
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
            //SliceLog.Log("Setting Textbox");

            if (regularTextObject.HasComponent<FontRenderer>())
            {
                //SliceLog.Log("Has Font");
                regularTextObject.GetComponent<FontRenderer>().Text_val = input;
            }
            else
            {
                //SliceLog.Log("No Font component");
            }
            //Set Text
        }

        public void SetName(string input)
        {
            //SliceLog.Log("Setting Namebox");

            if (nameTextObject.HasComponent<FontRenderer>())
            {
                //SliceLog.Log("Has Font");
                nameTextObject.GetComponent<FontRenderer>().Text_val = input;
            }
            else
            {
                //SliceLog.Log("No Font component");
            }
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
            //console.writeline("HUD Ini called");
            health = healthSliderObject.GetComponent<Slider>();      
            victory = victoryObject.GetComponent<SpriteRenderer>();
            defeat = defeatObject.GetComponent<SpriteRenderer>();
            LoadDialogues();
        }
    }
}

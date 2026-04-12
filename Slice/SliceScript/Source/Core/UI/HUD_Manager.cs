using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;

namespace SliceEngine
{
    public class HUD_Manager : SliceBehaviour, IInitializable
    {
        public bool DialogueOnStart = false;
        public int currLevel = 0; // to sync with level director
        public int currentScene = 0;
        public string nextSceneToLoad = "";
        public string currSceneToLoad = "";

        Slider health;
        public GameObject healthSliderObject;
        public GameObject healthBarObject;


        private GameObject defeatObject;
        //public GameObject retryBtn;

        public GameObject textBoxParentObject;
        public GameObject regularTextObject;
        public GameObject nameTextObject;
        public bool popUpOpen = false;

        //Letters per second
        public float typeSpeed = 5f;

        private bool enterPressed = false;
        private bool inputOpen = false;
        public bool dialogueDone = true;

        bool loseScreenOpen = false;

        public override void OnCreate()
        {
            //if (DialogueOnStart)
            //{
            //    PlayDialogueForLevel(currLevel);
            //    Bootstrap.Player.canInput = false;
            //}
            Cursor.state = Cursor.STATE.DISABLED;
            if(FindGameObjectWithName("LoseScreenFinal") != null)
            {
                defeatObject = FindGameObjectWithName("LoseScreenFinal");
            }
            else
            {
                defeatObject = FindGameObjectWithName("DefeatPlaceholder");
            }
                dialogueDone = true;
        }

        public override void OnUpdate(float dt)
        {
            //base.OnUpdate(dt);

            //if (inputOpen)
            //{
            //    if (Input.IsKeyDown(Keys.KEY_F) && enterPressed == false)
            //    {
            //        enterPressed = true;
            //        PlayDialogueForLevel(currLevel, currentScene);
            //    }

            //    if (Input.IsKeyReleased(Keys.KEY_F) && enterPressed == true)
            //    {
            //        enterPressed = false;
            //    }
            //}
        }


        public void SetHealth(float input)
        {
            //console.writeline("Setting health");
            //health.SetValue(input);
            healthSliderObject.GetComponent<Slider>().SetValue(input);
            //console.writeline("Finish setting health");
        }

        public void HideHUD(bool toHideOrNotToHide)
        {
            //Console.WriteLine("HIDING HUD");
            if (healthBarObject == null)
            {
                SliceLog.Error("No health bar assigned");
               // Console.WriteLine("No health bar obj");
                return;
            }
            
            float alpha = toHideOrNotToHide ? 1.0f : 0.0f;

            GameObject[] children = healthBarObject.GetAllChildren();

            //Console.WriteLine("DOASKODSAKDOAODAKOAKDOKA");

            Vector4 col = healthBarObject.GetComponent<SpriteRenderer>().Colour;
            col.w = alpha;
            healthBarObject.GetComponent<SpriteRenderer>().Colour = col;

            foreach (GameObject go in children)
            {
                if (go.HasComponent<SpriteRenderer>())
                {
                    col = go.GetComponent<SpriteRenderer>().Colour;
                    col.w = alpha;
                    go.GetComponent<SpriteRenderer>().Colour = col;
                }
                // theres no text in hud manager, so i dont have to include it
            }
        }

        //public void LoadNextLevel()
        //{
        //    SceneManager.LoadScene(nextSceneToLoad);
        //}

        //public void RestartLevel()
        //{
        //    SceneManager.LoadScene(currSceneToLoad);
        //}

        public void GameLoseScreen()
        {
            SliceLog.Console("Lost");
            //defeat.SetEnabled(true);
            if(defeatObject.HasComponent<SpriteRenderer>())
            {
                SpriteRenderer defeat = defeatObject.GetComponent<SpriteRenderer>();
                defeat.SetEnabled(true);
            }
            else
            {
                defeatObject.SetActive(true);

            }
            //retryBtn.SetActive(true);
            Cursor.state = Cursor.STATE.DEFAULT;
            loseScreenOpen = true;
        }

        public bool CheckLoseScreen()
        {
            if(loseScreenOpen)
            {
                return true;
            }

            return false;
        }

        public void CursorChecking(Cursor.STATE currentCursorState)
        {
            if (currentCursorState == Cursor.STATE.DISABLED)
            {
                Cursor.state = Cursor.STATE.DEFAULT;
            }
            else
            {
                Cursor.state = Cursor.STATE.DISABLED;
            }
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
        private SliceCSV popUpLoader = new SliceCSV();
        //Each entry currently needs name and text and index
        //Each set should have the index 

        //                Set      
        private Dictionary<string, List<string[]>> allDialogues = new Dictionary<string, List<string[]>>();

        public class PopupInfo
        {
            public string text;
            public string year;
            public string time;
            public string iteration;
        }

        public Dictionary<int, PopupInfo> allPopups = new Dictionary<int, PopupInfo>();
        public void LoadDialogues()
        {
            //Load dialogues from a CSV
            // SliceLog.Log("Streaming Assets filepath: " + Application.streamingAssetsPath);
            string filePath = Application.GetFilePath("Dialogue.csv");
            SliceLog.Log("Loading dialogue from, App filepath: " + filePath);
            loader.Load(filePath);

            if (File.Exists(filePath))
            {
                SliceLog.Log("Found dialogue from, App filepath: " + filePath);
            }
            else
            {
                SliceLog.Log("Could not find the Dialogues file from: " + filePath);
            }

            if (loader != null)
            {
                //Sorts and adds them to the specified sets

                for (int i = 0; i < loader.RowCount; i++)
                {
                    string sceneval = loader.GetValue(i, "Scene");
                    string setval = loader.GetValue(i, "Set");
                    string combinedKey = sceneval + "_" + setval;

                    if (!allDialogues.ContainsKey(combinedKey))
                    {
                        allDialogues.Add(combinedKey, new List<string[]>());

                    }
                    
                    allDialogues[combinedKey].Add(new string[] { loader.GetValue(i, "Name"), loader.GetValue(i, "Text"), loader.GetValue(i, "AudioFileName") });
                    //SliceLog.Log("Added dialogue entry with " + combinedKey);
                }
            }
        }

        public void LoadPopups()
        {
            string filePath = Application.GetFilePath("Popup.csv");
            SliceLog.Log("Loading dialogue from, App filepath: " + filePath);
            popUpLoader.Load(filePath);

            if (File.Exists(filePath))
            {
                SliceLog.Log("Found pop ups from, App filepath: " + filePath);
            }
            else
            {
                SliceLog.Log("Could not find the pop up file from: " + filePath);
            }

            if (popUpLoader != null)
            {
                for (int i = 0; i < popUpLoader.RowCount; i++)
                {
                    int index = popUpLoader.GetValue<int>(i, "Index");
                    PopupInfo info = new PopupInfo();

                    info.text = popUpLoader.GetValue<string>(i, "Text");
                    info.year = popUpLoader.GetValue<string>(i, "Year");
                    info.time = popUpLoader.GetValue<string>(i, "Time");
                    info.iteration = popUpLoader.GetValue<string>(i, "Iteration");
                    allPopups.Add(index, info);
                }
            
            }
        }

        private bool typing = false;

        //string[] for listed things 0 = name, 1 = text
        //private List<string[]> levelDialogues = new List<string[]>();

        public int dialogueIndex = 0;

        public bool PlayDialogueForLevel(int level, int scene, bool locksCamera, bool locksControls)
        {

            // Skip to display full line when type writer effect is playing.
            if (typing == true)
            {
                typing = false;
                return true;
            }


            //Close dialogue box if it is the last line of the set
            if (!allDialogues.ContainsKey(scene + "_" + level) || (allDialogues[scene+"_"+level].Count == dialogueIndex + 1 && dialogueDone == false))
            {
                // end of dialogue stack
                // clear stack
                SliceLog.Log("failed to find dialogue or out of range for the key: " + scene + "_" + level);
                currLevel++; // increment curr level to prevent reloading same dialogue set
                inputOpen = false;
                dialogueDone = true;
                //Bootstrap.Player.canInput = true;
                dialogueIndex = -1;
                //levelDialogues.Clear();
                SetTextBox("");
                CloseTextBox();
                return false;
            }


            // Will tick dialogue up if it is already loaded, else will load fresh set and play
            if (allDialogues[scene + "_" + level].Count > 0)
            {
                // dialogues is not empty
                //  tick up number

                SliceLog.Log("Dialogue is set to:" + dialogueDone);

                if (dialogueDone)
                {
                    SliceLog.Log("Dialogue Is fresh. Setting index to 0");
                    dialogueIndex = 0;
                    dialogueDone= false;
                }
                else
                {
                    SliceLog.Log("Incrementing Dialogue");
                    dialogueIndex++;
                }

                    SliceLog.Log("Dialogue is not empty");

            }
            else
            {


                return true;
                SliceLog.Log("Dialogue is empty");


                /*
                //Loading from the list
                for (int i = loader.FindRowIndex("Set", level.ToString()); i > -1; i++)
                {
                    //SliceLog.Log("index is at" + i);

                    if (loader.GetValue(i, "Set") != level.ToString())
                    {
                        break;
                    }
                    else if (int.Parse(loader.GetValue(i, "Set")) < currLevel)
                    {
                        break;
                    }

                    dialogueDone = false;

                    levelDialogues.Add(new string[] { loader.GetValue(i, "Name"), loader.GetValue(i, "Text") });
                }
                */
            }


            OpenTextBox();
            typing = true;
            StartCoroutine(TypeText(allDialogues[scene + "_" + level][dialogueIndex][1], allDialogues[scene + "_" + level][dialogueIndex][2]));
            SetName(allDialogues[scene + "_" + level][dialogueIndex][0]);
            inputOpen = true;
            currLevel = level;
            return true;
        }

        IEnumerator TypeText(string toType, string audioToPlay)
        {
            //float timecounter = 0f;
            float speed = 1f / typeSpeed;
            string displaying = "";

            AudioSettings.PlaySFX(audioToPlay);

            SliceLog.Log("To type is:" + toType);
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
            typing = false;


            yield break;
        }



        #region Textbox controls


        public void SetTextBox(string input)
        {
            //SliceLog.Log("Setting Textbox");

            if (regularTextObject.HasComponent<FontRenderer>())
            {
                //SliceLog.Log("Has Font");
                regularTextObject.GetComponent<FontRenderer>().Text_val = input.ToUpper();
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
                nameTextObject.GetComponent<FontRenderer>().Text_val = input.ToUpper();
            }
            else
            {
                //SliceLog.Log("No Font component");
            }
        }

        public void OpenTextBox()
        {
            SliceLog.Log("Open Text Box called");
            Bootstrap.Player.SetPlayerLock(true);
            Bootstrap.CameraController.LockCamera = true;
            textBoxParentObject.SetActive(true);
            Cursor.state = Cursor.STATE.DISABLED;
        }

        public void CloseTextBox()
        {
            SliceLog.Log("Open Text Box called");
            Bootstrap.Player.SetPlayerLock(false);
            Bootstrap.CameraController.LockCamera = false;
            textBoxParentObject.SetActive(false);
            Cursor.state = Cursor.STATE.DISABLED;
        }

        #endregion

        public void Initialize()
        {
            //console.writeline("HUD Ini called");
            health = healthSliderObject.GetComponent<Slider>();
            //defeat = defeatObject.GetComponent<SpriteRenderer>();
            LoadDialogues();
            LoadPopups();
            //Input.SetCursorState(Cursor.STATE.HIDDEN);
        }
    }
}
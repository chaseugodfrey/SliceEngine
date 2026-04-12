using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
using System.Collections.Generic;
using System.IO;

namespace SliceEngine
{
    public class PopupTriggerBox : SliceBehaviour
    {
        public string dialogueBoxObject = "PopUp";
        public GameObject dialogueBox;

        public GameObject iterationText;
        public GameObject yearText;
        public GameObject timeText;
        public GameObject contentText;

        public bool triggered = false;
        public int popupIndex = 0;
        public bool typing = false;
        public float typeSpeed = 30f;
        

        public override void OnCreate()
        {
            if (dialogueBoxObject != string.Empty)
            {
                GameObject[] gameObjects = gameObject.FindGameObjectsWithTag(dialogueBoxObject);
                if (gameObjects == null)
                {
                    SliceLog.Error("No gamobject with tag " + dialogueBoxObject);
                    return;
                }

                dialogueBox = gameObjects[0];
                iterationText = gameObject.FindGameObjectWithTag("PopUpIteration");
                yearText = gameObject.FindGameObjectWithTag("PopUpYear");
                timeText = gameObject.FindGameObjectWithTag("PopUpTime");
                contentText = gameObject.FindGameObjectWithTag("PopUpContent");

                if (iterationText == null)
                {
                    SliceLog.Error("iteration text tag not set");
                }
                if (yearText == null)
                {
                    SliceLog.Error("yearText tag not set");
                }
                if (timeText == null)
                {
                    SliceLog.Error("timeText tag not set");
                }
                if (contentText == null)
                {
                    SliceLog.Error("contentText tag not set");
                }


                if (dialogueBox == null)
                {
                    SliceLog.Error("idk how it reached here tbh");
                }
            }
        }

        public void SetTextBox(string input)
        {
            if (contentText != null)
            {
                if (contentText.HasComponent<FontRenderer>())
                {
                    contentText.GetComponent<FontRenderer>().Text_val = input.ToUpper();
                }
            }
        }

        public override void OnUpdate(float dt)
        {
            // there is a known bug that i can foresee but i kinda lazy to deal w now
            // if the player presses F in the trigger box while game settings is open
            // it might have a funky iteraction
            if (triggered)
            {
                // skip the slow typing thing
                if (Input.IsKeyPressed(Keys.KEY_F))
                {
                    typing = false;
                }

                if (Input.IsKeyPressed(Keys.KEY_ESC) && Bootstrap.HUDManager.CheckLoseScreen() == false)
                {
                    dialogueBox.SetActive(false);
                    StartCoroutine(closeTimer());
                    //triggered = false;
                }
            }
        }
        IEnumerator closeTimer()
        {
            float elapsedTime = 0.0f;

            while (elapsedTime < 0.1f)
            {
                elapsedTime += Time.deltaTime;

                yield return null;
            }

            Bootstrap.HUDManager.popUpOpen = false;
            triggered = false;
            SetTextBox("");
        }

        IEnumerator TypeText(string toType)
        {
            float speed = 1f / typeSpeed;
            string displaying = "";
            for(int i = 0; i < toType.Length; ++i)
            {
                if (typing == false)
                    break;

                displaying += toType[i];

                SetTextBox(displaying);

                yield return new WaitForSeconds(speed);
            }

            SetTextBox(toType);
            typing = false;

            yield break;
        }
        public override void OnTriggerStay(uint other)
        {
            GameObject hit = gameObject.FindGameObjectWithID(other);
            if (hit.Has<PlayerController>() && Bootstrap.Player == hit.As<PlayerController>() && !triggered)
            {
                if (Input.IsKeyPressed(Keys.KEY_F))
                {
                    if (dialogueBox != null)
                    {
                        triggered = true;
                        // set up dialogue box here
                        dialogueBox.SetActive(true);

                        if (popupIndex >= Bootstrap.HUDManager.allPopups.Count)
                        {
                            SliceLog.Error("Out of index popup index");
                            return;
                        }

                        iterationText.GetComponent<FontRenderer>().Text_val = Bootstrap.HUDManager.allPopups[popupIndex].iteration.ToUpper();
                        yearText.GetComponent<FontRenderer>().Text_val = Bootstrap.HUDManager.allPopups[popupIndex].year;
                        timeText.GetComponent<FontRenderer>().Text_val = Bootstrap.HUDManager.allPopups[popupIndex].time;
                        typing = true;
                        SetTextBox("");
                        Bootstrap.HUDManager.popUpOpen = true;
                        StartCoroutine(TypeText(Bootstrap.HUDManager.allPopups[popupIndex].text));
                    }
                }
            }
        }
    }
}

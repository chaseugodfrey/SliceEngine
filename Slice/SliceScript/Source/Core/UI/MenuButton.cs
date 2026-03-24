using System;
using SliceEngine;
using SliceScript.Source.Core.Systems;

namespace SliceEngine
{
    public class MenuButton : SliceBehaviour
    {
        public int buttonType = 0;
        public string sceneToLoad = "";

        private MainMenuController mainController;
        private GameSettings gameSettingsController;
        private UIAnimation uiAnimController;
        
        private AudioSource btnAudio;
        public GameObject uiAnimObj;
        



        public override void OnCreate()
        {
            
            GameObject controllerObj = FindGameObjectWithName("MainMenu_Canvas");

            if (controllerObj != null)
            {
                mainController = controllerObj.As<MainMenuController>();
            }

            GameObject settingsObj = FindGameObjectWithName("GameManager");
            if (settingsObj != null)
            {
                gameSettingsController = settingsObj.As<GameSettings>();
            }

            GameObject audioObj = FindGameObjectWithName("MainMenu_Sfx");
            if (audioObj != null)
            {
                btnAudio = audioObj.GetComponent<AudioSource>();
            }

            

            //GameObject startRect = FindGameObjectWithName("OverlayGameRect");
            //if (startRect != null)
            //{
            //    testTrans = startRect.GetComponent<RectTransform>();

            //}

            if (uiAnimObj != null)
            {
                uiAnimController = uiAnimObj.As<UIAnimation>();

            }

            

        }

        public override void OnUpdate(float dt)
        {
            
            
        }

        public override void OnButtonClick()
        {
            if(uiAnimController!=null)
            {
                
                uiAnimController.ButtonClickAnim();

            }

            if (mainController != null)
            {
                if (buttonType == 0)
                {
                    mainController.StartGame(sceneToLoad);
                }
                else if (buttonType == 1)
                {
                    mainController.OpenSettings();
                    if (uiAnimController != null)
                    {
                        uiAnimController.ButtonHoverState(false);
                        uiAnimController.ResetButton();

                    }

                  
                }
                else if (buttonType == 2)
                {
                    mainController.QuitGame();
                }
                else if (buttonType == 3)
                {
                    mainController.CloseSettings();
                    if (uiAnimController != null)
                    {
                        uiAnimController.ButtonHoverState(false);
                        uiAnimController.ResetButton();

                    }
                }
                else if (buttonType == 4)
                {
                    mainController.BackToMenu();

                }
                else if (buttonType == 5)
                {
                    SliceLog.Log("Test");
                }
            }

            // --- GAME SCENE ACTIONS ---
            if (gameSettingsController != null)
            {
                if (buttonType == 5) // Resume
                {
                    gameSettingsController.ResumeGame();
                    if (uiAnimController != null)
                    {
                        uiAnimController.ButtonHoverState(false);
                        uiAnimController.ResetButton();

                    }
                }
                else if (buttonType == 1) // Open Settings (Sub-menu)
                {
                    gameSettingsController.OpenSubSettings();
                    if (uiAnimController != null)
                    {
                        uiAnimController.ButtonHoverState(false);
                        uiAnimController.ResetButton();

                    }
                }
                else if (buttonType == 3) // Close Settings (Back button inside popup)
                {
                    gameSettingsController.CloseSubSettings();
                    if (uiAnimController != null)
                    {
                        uiAnimController.ButtonHoverState(false);
                        uiAnimController.ResetButton();

                    }
                }
            }

            if (buttonType == 4)
            {
                SceneManager.LoadScene("MenuScene");
            }
        }

        public override void OnButtonHover()
        {
            if(uiAnimController != null)
            {
            
                uiAnimController.ButtonHoverState(true);

            }
        }

        public override void OnButtonExitHover()
        {
            if (uiAnimController != null)
            {

                uiAnimController.ButtonHoverState(false);

            }
        }

        public override void OnButtonRelease()
        {
            
        }

        
    }
}
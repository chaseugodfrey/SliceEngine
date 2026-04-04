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
        
        
        private AudioSource btnAudio;
        private FontRenderer fontComp;
        



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

            fontComp = GetComponent<FontRenderer>();
            

        }

        public override void OnUpdate(float dt)
        {
            
            
        }

        public override void OnButtonClick()
        {

            AudioSettings.PlaySFX("UIClick");

            if (mainController != null)
            {
                if (buttonType == 0)
                {
                    mainController.StartGame(sceneToLoad);
                }
                else if (buttonType == 1)
                {
                    mainController.OpenSettings();


                }
                else if (buttonType == 2)
                {
                    mainController.QuitGame();
                }

                else if (buttonType == 3)
                {
                    mainController.CloseSettings();

                }
                //else if (buttonType == 4)
                //{
                //    mainController.BackToMenu();

                //}

                
                
            }

            
            else if (gameSettingsController != null)
            {
                if (buttonType == 5) // Resume
                {
                    gameSettingsController.ResumeGame();

                }

                else if(buttonType == 8)
                {
                    Bootstrap.LevelDirector.LoadNextLevel();
                }else if(buttonType == 9)
                {
                    Bootstrap.LevelDirector.RestartLevel();
                }
                //else if (buttonType == 1) // Open Settings (Sub-menu)
                //{
                //    gameSettingsController.OpenSubSettings();

                //}
                //else if (buttonType == 3) // Close Settings (Back button inside popup)
                //{
                //    gameSettingsController.CloseSubSettings();

                //}
            }

            if (buttonType == 4) //
            {
                SceneManager.LoadScene("MenuScene");
                gameSettingsController.ResumeGame();
                
            }

            else if(buttonType == 6) //AudioPage in Settings
            {
                if (mainController != null) mainController.ToggleSettingsPages(true);
                if (gameSettingsController != null) gameSettingsController.ToggleSettingsPages(true);
            }

            else if (buttonType == 7) //GraphicsPage in Settings
            {
                if (mainController != null) mainController.ToggleSettingsPages(false);
                if (gameSettingsController != null) gameSettingsController.ToggleSettingsPages(false);
            }

            else if(buttonType == 10)
            {
                if (mainController != null)
                {
                    mainController.RestoreDefaults();
                }else if(gameSettingsController!=  null)
                {
                    gameSettingsController.RestoreDefaults();
                }
            }
        }

        public override void OnButtonHover()
        {
            if(buttonType == 0 || buttonType == 1 || buttonType == 2 ||buttonType ==10)
            {
                fontComp.Colour = new Vector4(203.0f *(1.0f /256.0f), 203.0f * (1.0f / 256.0f), 203.0f * (1.0f / 256.0f), 1.0f);
            }
        }

        public override void OnButtonExitHover()
        {
            if (buttonType == 0 || buttonType == 1 || buttonType == 2 || buttonType == 10)
            {
                fontComp.Colour = new Vector4(1.0f,1.0f,1.0f,1.0f);
            }
        }

        public override void OnButtonRelease()
        {
            
        }

        
    }
}
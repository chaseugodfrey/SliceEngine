using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;
using System.Runtime.InteropServices;


namespace SliceEngine
{
    public class MainMenuController : SliceBehaviour
    {

        private GameObject settingsPopup;
        private GameObject bgAnimationObject;
        private GameObject MainMenuCanvas;
        
        private SettingsBorderAnimation borderAnim;
        private SettingsBGAnimation bgAnim;

        private bool isSettingsOpen = false;
        

        public string textToShow = "";
        

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup_Final");
            bgAnimationObject = FindGameObjectWithName("SettingsBGSpriteSheet");
            MainMenuCanvas = FindGameObjectWithName("MainMenu_Canvas");

            if (settingsPopup != null)
            {
                borderAnim = settingsPopup.As<SettingsBorderAnimation>();
                if (borderAnim != null)
                {
                    borderAnim.coreElements[1] = MainMenuCanvas;
                    borderAnim.pages[0] = FindGameObjectWithName("AudioSettingsPage");
                    borderAnim.pages[1] = FindGameObjectWithName("GraphicsSettingsPage");

                    borderAnim.coreElements[4] = FindGameObjectWithName("ReturnToTitleButton");

                    borderAnim.titleElements[0] = FindGameObjectWithName("MiniTitleText");
                }
                settingsPopup.SetActive(false);
            }

            if (bgAnimationObject != null)
            {
                bgAnim = bgAnimationObject.As<SettingsBGAnimation>();
                if (bgAnim != null)
                {
                    bgAnim.borderAnim = borderAnim;
                }
                bgAnimationObject.SetActive(false);
            }
        }

        public override void OnUpdate(float dt)
        {
            if(Input.IsKeyPressed(Keys.KEY_ESC))
            {
                if(isSettingsOpen)
                {
                    CloseSettings();
                }
            }
        }

        public void ToggleSettingsPages(bool isAudio)
        {
            if (borderAnim != null) borderAnim.SwitchToPage(isAudio);
        }

        public void StartGame(string sceneName)
        {
            if (sceneName != "")
            {
                SceneManager.LoadScene(sceneName);
            }
        }

        public void BackToMenu()
        {
            SceneManager.LoadScene("MenuScene");
        }

        public void QuitGame()
        {
            SceneManager.QuitGame();
        }

        public void OpenSettings()
        {
            isSettingsOpen = true;

            if (bgAnim != null)
            {
                bgAnim.StartSettingsBGAnimation(true);
            }
            else if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(true);
            }

            if (MainMenuCanvas != null) MainMenuCanvas.SetActive(false);
        }

        public void CloseSettings()
        {
            isSettingsOpen = false;

            if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(false);
                AudioSettings.PlaySFX("PauseTransitionOut");
            }
        }


    }
}

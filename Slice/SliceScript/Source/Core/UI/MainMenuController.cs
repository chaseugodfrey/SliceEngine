using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;


namespace SliceEngine
{
    public class MainMenuController : SliceBehaviour
    {

        private GameObject settingsPopup;
        private GameObject bgAnimationObject;
        private GameObject MainMenuCanvas;
        private SettingsBorderAnimation borderAnim;
        private SettingsBGAnimation bgAnim;

        private float animationTimer = 0f;

        public string textToShow = "";
        

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup");
            bgAnimationObject = FindGameObjectWithName("SettingsBGSpriteSheet");
            MainMenuCanvas = FindGameObjectWithName("MainMenu_Canvas");

            if (settingsPopup != null)
            {
                borderAnim = settingsPopup.As<SettingsBorderAnimation>();
                if (borderAnim != null)
                {
                    borderAnim.menuCanvasObj = MainMenuCanvas;
                    borderAnim.audioSettingsPage = FindGameObjectWithName("AudioSettingsPage");
                    borderAnim.graphicsSettingsPage = FindGameObjectWithName("GraphicsSettingsPage");
                    //borderAnim.audioButton = FindGameObjectWithName("AudioButton");
                    //borderAnim.graphicsButton = FindGameObjectWithName("GraphicsButton");
                    borderAnim.returnToTitleButton = FindGameObjectWithName("ReturnToTitleButton");
                    borderAnim.miniTitleTextObj = FindGameObjectWithName("MiniTitleText");
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
            if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(false);
            }
        }


    }
}

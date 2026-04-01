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

            if (bgAnimationObject != null)
            {
                // Access the script we just built
                bgAnimationObject.SetActive(false);
                borderAnim = settingsPopup.As<SettingsBorderAnimation>();
                bgAnim = bgAnimationObject.As<SettingsBGAnimation>();
                
            }


        }

        public override void OnUpdate(float dt)
        {
            
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
                
                //bgAnimationObject.SetActive(true);
                //bgAnim.StartSettingsBGAnimation(true);
                borderAnim.StartSettingsPopupAnimation(true);
            }
            //if (MainMenuCanvas != null) MainMenuCanvas.SetActive(false);
        }

        public void CloseSettings()
        {
            //if (settingsPopup != null) settingsPopup.SetActive(false);
            if (borderAnim != null)
            {
                bgAnimationObject.SetActive(false);
                borderAnim.StartSettingsPopupAnimation(false);
                
            }
            //if(MainMenuCanvas != null) MainMenuCanvas.SetActive(true);
        }


    }
}
using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;


namespace SliceEngine
{
    public class MainMenuController : SliceBehaviour
    {

        private GameObject settingsPopup; // The object holding the Animation script
        private GameObject MainMenuCanvas;
        private SettingsBorderAnimation borderAnim;

        private float animationTimer = 0f;

        public string textToShow = "";
        private SettingsBorderAnimation uiBorderAnimController;

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup");
            MainMenuCanvas = FindGameObjectWithName("MainMenu_Canvas");

            if (settingsPopup != null)
            {
                // Access the script we just built
                borderAnim = settingsPopup.As<SettingsBorderAnimation>();
                settingsPopup.SetActive(false);
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

            if (borderAnim != null)
            {
                borderAnim.StartAnimation(true);
            }
            if (MainMenuCanvas != null) MainMenuCanvas.SetActive(false);
        }

        public void CloseSettings()
        {
            //if (settingsPopup != null) settingsPopup.SetActive(false);
            if (borderAnim != null)
            {
                borderAnim.StartAnimation(false);
            }
            //if(MainMenuCanvas != null) MainMenuCanvas.SetActive(true);
        }


    }
}
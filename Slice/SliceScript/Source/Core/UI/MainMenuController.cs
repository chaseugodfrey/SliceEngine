using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;


namespace SliceEngine
{
    public class MainMenuController : SliceBehaviour
    {

        private GameObject settingsPopup;
        private GameObject MainMenuCanvas;

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup");
            MainMenuCanvas = FindGameObjectWithName("MainMenu_Canvas");

            if (settingsPopup != null)
            {
                settingsPopup.SetActive(false);
            }


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
            if (settingsPopup != null) settingsPopup.SetActive(true);
            
            if(MainMenuCanvas != null) MainMenuCanvas.SetActive(false);
        }

        public void CloseSettings()
        {
            if (settingsPopup != null) settingsPopup.SetActive(false);
            if(MainMenuCanvas != null) MainMenuCanvas.SetActive(true);
        }


    }
}
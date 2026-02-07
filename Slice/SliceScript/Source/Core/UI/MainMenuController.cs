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


            if (MainMenuCanvas != null) 
            {
                MainMenuCanvas.SetActive(true); 
            }
        }

        public void StartGame(string sceneName)
        {
            if (!string.IsNullOrEmpty(sceneName))
            {
                SceneManager.LoadScene(sceneName);
            }
            else
            {
                SliceLog.Log("MainMenuController: No scene name provided for StartGame!");
            }
        }

        public void QuitGame()
        {
            SceneManager.QuitGame();
        }

        public void ToggleSettings(bool isOpen)
        {
            if (settingsPopup != null) 
            { 
                settingsPopup.SetActive(isOpen); 
            }
            if (MainMenuCanvas != null) 
            { 
                MainMenuCanvas.SetActive(!isOpen); 
            }
        }

        
    }
}
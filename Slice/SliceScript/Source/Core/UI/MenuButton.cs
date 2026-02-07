using SliceEngine;
using SliceScript.Source.Core.Systems;

namespace SliceEngine
{
    public class MenuButton : SliceBehaviour
    {
        public int buttonType = 0;
        public string sceneToLoad = "";

        private MainMenuController controller;
        private AudioSource btnAudio;

        public override void OnCreate()
        {
            
            GameObject controllerObj = FindGameObjectWithName("MainMenu_Canvas");

            if (controllerObj != null)
            {
                controller = controllerObj.As<MainMenuController>();
            }

            GameObject audioObj = FindGameObjectWithName("MainMenu_Sfx");
            if (audioObj != null)
            {
                btnAudio = audioObj.GetComponent<AudioSource>();
            }
        }

        public override void OnButtonClick()
        {
            if (controller == null) 
            { 
                return; 
            }

            if (buttonType == 0) // Play
            {
                controller.StartGame(sceneToLoad);
            }
            else if (buttonType == 1) // Settings
            {
                controller.OpenSettings();
            }
            else if (buttonType == 2) // Quit
            {
                controller.QuitGame();
            }
            else if (buttonType == 3) // Close Settings
            {
                controller.CloseSettings();
            }
        }

        public override void OnButtonRelease()
        {

            if (btnAudio != null)
            {
                btnAudio.Play();
            }
        }
    }
}
using SliceEngine;
using SliceScript.Source.Core.Systems;

namespace SliceEngine
{
    public enum ButtonType
    {
        Play,
        OpenSettings,
        CloseSettings,
        Quit
    }

    public class MenuButton : SliceBehaviour
    {
        public ButtonType type;
        public string sceneToLoad = "";

        private MainMenuController controller;
        private AudioSource buttonSfx;

        public override void OnCreate()
        {
            
            GameObject controllerObj = FindGameObjectWithName("MainMenu_Canvas");
            if (controllerObj != null)
            {
                controller = controllerObj.GetComponent<MainMenuController>();
            }
            else
            {
                SliceLog.Log("MenuButton: Could not find MainMenuController!");
            }

            GameObject audioObj = FindGameObjectWithName("MainMenu_Sfx");
            if (audioObj != null)
            {
                buttonSfx = audioObj.GetComponent<AudioSource>();
            }
        }

        public override void OnButtonClick()
        {
            if (controller == null) 
            { 
                return; 
            }

            switch (type)
            {
                case ButtonType.Play:
                    controller.StartGame(sceneToLoad);
                    break;

                case ButtonType.OpenSettings:
                    controller.ToggleSettings(true);
                    break;

                case ButtonType.CloseSettings:
                    controller.ToggleSettings(false);
                    break;

                case ButtonType.Quit:
                    controller.QuitGame();
                    break;
            }
        }

        public override void OnButtonRelease()
        {
            
            if (buttonSfx != null)
            {
                buttonSfx.Play();
            }
        }
    }
}
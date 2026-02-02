using SliceEngine;
using System;


namespace SliceEngine
{
    public class MainMenuButtons : SliceBehaviour
    {
        public float isPlay = 1.0f;

        public override void OnButtonClick()
        {
            Console.WriteLine("Main menu is being pressed");
        }

        public override void OnButtonRelease()
        {
            AudioSource myAudio = gameObject.FindGameObjectWithName("MainMenu_Sfx").GetComponent<AudioSource>();

            Console.WriteLine("Main menu is being released");

            if (isPlay > 0.5f)
            {
                gameObject.FindGameObjectWithName("MainMenu_Cam").As<MainMenuCamera>().ActivateMainMenuCamera();
                if (myAudio != null)
                {
                    myAudio.Play();
                    SliceLog.Log("MainMenu Audio");
                }
                SliceLog.Log("Click Play");
                gameObject.FindGameObjectWithName("MainMenu_Canvas").Destroy();
               // Bootstrap.LevelDirector.StartGame();
                Bootstrap.Player.canInput = true;
                Bootstrap.Player.canMove = true;
            }
            else
            {
                if (myAudio != null)
                {
                    myAudio.Play();
                    SliceLog.Log("MainMenu Audio");
                }
                SliceLog.Log("Click Exit");
            }
        }
    }
}
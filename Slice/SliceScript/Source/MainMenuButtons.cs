using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;


namespace SliceEngine
{
    public class MainMenuButtons : SliceBehaviour
    {
        public bool isPlay = false;
        public bool isSettings = false;
        public bool isQuit = false;
        public bool isSettingsPopup = false;
        public bool isMaxVolumeSlider = false;
        public bool isSFXVolumeSlider = false;
        public bool isBGMVolumeSlider = false;

        GameObject settingsPopup;
        GameObject MainMenuCanvas;
        GameObject BGMSlider;
        GameObject SFXSlider;
        GameObject MasterSlider;
        Slider bgmVol;
        Slider sfxVol;
        Slider masterVol;

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup");
            MainMenuCanvas = FindGameObjectWithName("MainMenu_Canvas");
            BGMSlider = FindGameObjectWithName("BGMVolumeSlider");
            SFXSlider = FindGameObjectWithName("SFXVolumeSlider");
            MasterSlider = FindGameObjectWithName("MasterVolumeSlider");

            bgmVol = BGMSlider.GetComponent<Slider>();
            sfxVol = BGMSlider.GetComponent<Slider>();
            masterVol = BGMSlider.GetComponent<Slider>();
        }

        public override void OnUpdate(float dt)
        {
            //if (isSFXVolumeSlider)
            //{

            //    AudioManager.SetCategoryVolume("SFX", value);
            //}
            //else if (isMaxVolumeSlider)
            //{
            //    AudioManager.SetMasterVolume(value);
            //}
            //else if (isBGMVolumeSlider)
            //{
            //    AudioManager.SetCategoryVolume("BGM", value);
            //    Console.WriteLine("BGM volume");
            //}

           

            
        }

        public override void OnButtonClick()
        {
            Console.WriteLine("Main menu is being pressed");

            //Lerp thingy here

            if(isPlay)
            {
                SceneManager.LoadScene("Gid_Working_Scene");
                Console.WriteLine("LOad");
            }else if(isSettings)
            {
                ShowSettingsPopup();
                Console.WriteLine("Settings pop");
            }else if(isQuit)
            {
                SceneManager.QuitGame();
                Console.WriteLine("Bye Bye");
            }else if(isSettingsPopup)
            {
                HideSettingsPopup();
                Console.WriteLine("Settings bye");
            }
        }

        public override void OnSliderValue(float value)
        {
            float bgmCurrVol = bgmVol.GetValue();
            float sfxCurrVol = sfxVol.GetValue();
            float masterCurrVol = masterVol.GetValue();
            if (bgmCurrVol != AudioManager.GetCategoryVolume("BGM"))
            {
                AudioManager.SetCategoryVolume("BGM", bgmVol.GetValue());
            }
            if (sfxCurrVol != AudioManager.GetCategoryVolume("SFX"))
            {
                AudioManager.SetCategoryVolume("SFX", sfxVol.GetValue());
            }

            if (masterCurrVol != AudioManager.GetMasterVolume())
            {
                AudioManager.SetMasterVolume(masterVol.GetValue());
            }
        }

        public override void OnButtonRelease()
        {
            AudioSource myAudio = gameObject.FindGameObjectWithName("MainMenu_Sfx").GetComponent<AudioSource>();

            Console.WriteLine("Main menu is being released");

            //if (isPlay > 0.5f)
            //{
            //    gameObject.FindGameObjectWithName("MainMenu_Cam").As<MainMenuCamera>().ActivateMainMenuCamera();
            //    if (myAudio != null)
            //    {
            //        myAudio.Play();
            //        SliceLog.Log("MainMenu Audio");
            //    }
            //    SliceLog.Log("Click Play");
            //    gameObject.FindGameObjectWithName("MainMenu_Canvas").Destroy();
            //   // Bootstrap.LevelDirector.StartGame();
            //    Bootstrap.Player.canInput = true;
            //    Bootstrap.Player.canMove = true;
            //}
            //else
            //{
            //    if (myAudio != null)
            //    {
            //        myAudio.Play();
            //        SliceLog.Log("MainMenu Audio");
            //    }
            //    SliceLog.Log("Click Exit");
            //}

            if (myAudio != null)
            {
                myAudio.Play();
                SliceLog.Log("MainMenu Audio");
            }
        }

        public void ShowSettingsPopup()
        {
            settingsPopup.SetActive(true);
            MainMenuCanvas.SetActive(false);
        }

        public void HideSettingsPopup()
        {
            settingsPopup.SetActive(false);
            MainMenuCanvas.SetActive(true);
        }
    }
}
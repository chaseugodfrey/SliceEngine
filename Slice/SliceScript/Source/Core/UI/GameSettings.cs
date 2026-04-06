using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class GameSettings : SliceBehaviour
    {

        private GameObject settingsPopup;
        private GameObject bgAnimationObject;
        private GameObject beforeGammaImage;
        private GameObject pauseBGM;
        private GameObject levelBGM;

        private SettingsBorderAnimation borderAnim;
        private SettingsBGAnimation bgAnim;

        private AudioSource pauseAudioSource;
        private AudioSource levelAudioSource;
        //private PreferenceSettings preferenceSettings;

        private SpriteRendererGammaOverride spriteGammaOverride;

        private bool isSettingsOpen = false;

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup_Levels");
            bgAnimationObject = FindGameObjectWithName("SettingsBGSpriteSheet");
            beforeGammaImage = FindGameObjectWithName("BeforeImage");

            if(FindGameObjectWithName("PauseMusic") != null)
            {
            
                pauseBGM = FindGameObjectWithName("PauseMusic");

            }

            if(FindGameObjectWithName("LevelBGM") != null)
            {
            
                levelBGM = FindGameObjectWithName("LevelBGM");

            }



            if (settingsPopup != null)
            {
                borderAnim = settingsPopup.As<SettingsBorderAnimation>();
                
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

            if(beforeGammaImage!= null)
            {
                spriteGammaOverride = beforeGammaImage.GetComponent<SpriteRendererGammaOverride>();
            }

            if (pauseBGM != null)
            {
                pauseAudioSource = pauseBGM.GetComponent<AudioSource>();
            }

            if (levelBGM != null)
            {
                levelAudioSource = levelBGM.GetComponent<AudioSource>();
            }



            PreferenceSettings.Initialize();

            spriteGammaOverride.Gamma = Camera.Gamma;

            isSettingsOpen = false;
        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_ESC) && Bootstrap.HUDManager.CheckLoseScreen() == false)
            {
                if (isSettingsOpen)
                {
                    ResumeGame();
                }
                else
                {
                    OpenSubSettings();
                }
            }
        }

        public void ToggleSettingsPages(bool isAudio)
        {
            if (borderAnim != null) borderAnim.SwitchToPage(isAudio);
        }



        public void ResumeGame()
        {
            isSettingsOpen = false;

            PreferenceSettings.SavePreferences();

            spriteGammaOverride.Gamma = Camera.Gamma;

            if (pauseAudioSource != null)
            {
                pauseAudioSource.Stop();
            }

            if (levelAudioSource != null)
            {
                levelAudioSource.IsPaused = false;
            }

            if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(false);
                AudioSettings.PlaySFX("PauseTransitionOut");
            }

            // Force close everything
            //if (settingsPopup != null) settingsPopup.SetActive(false);

            settingsPopup.SetActive(false);


            SliceLog.Console("Resume");

            Cursor.state = Cursor.STATE.DISABLED;
            Time.timeScale = 1.0f;
        }

        public void RestoreDefaults()
        {

            PreferenceSettings.RestoreDefaults();

            //spriteGammaOverride.Gamma = Camera.Gamma * 10.0f;

            if (borderAnim != null)
            {
                borderAnim.SyncSlidersToEngine();
            }
        }

        public void OpenSubSettings()
        {

            isSettingsOpen = true;

            if (levelAudioSource != null)
            {
                levelAudioSource.IsPaused = true;
            }

            if (pauseAudioSource != null)
            {
                pauseAudioSource.Play();
            }

            if (bgAnim != null)
            {
                AudioSettings.PlaySFX("PauseTransitionIn");
                bgAnim.StartSettingsBGAnimation(true);
            }
            else if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(true);
            }


            Cursor.state = Cursor.STATE.DEFAULT;
            Time.timeScale = 0.0f;
        }

        //public void CloseSubSettings()
        //{
        //    if (borderAnim != null)
        //    {
        //        borderAnim.StartSettingsPopupAnimation(false);
        //    }

        //    isSubSettingsOpen = false;
        //    isPauseMenuOpen = true;
        //}
    }
}

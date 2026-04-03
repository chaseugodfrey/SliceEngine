using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;
using System.Runtime.InteropServices;


namespace SliceEngine
{
    public class MainMenuController : SliceBehaviour
    {

        private GameObject settingsPopup;
        private GameObject bgAnimationObject;
        private GameObject MainMenuCanvas;
        private GameObject beforeGammaImage;

        private SettingsBorderAnimation borderAnim;
        private SettingsBGAnimation bgAnim;
        //private PreferenceSettings preferenceSettings;

        private SpriteRendererGammaOverride spriteGammaOverride;

        private bool isSettingsOpen = false;
        private float animationTimer = 0f;

        public string textToShow = "";


        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup_Final");
            bgAnimationObject = FindGameObjectWithName("SettingsBGSpriteSheet");
            MainMenuCanvas = FindGameObjectWithName("MainMenu_Canvas");
            beforeGammaImage = FindGameObjectWithName("BeforeImage");


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

            if (beforeGammaImage != null)
            {
                spriteGammaOverride = beforeGammaImage.GetComponent<SpriteRendererGammaOverride>();
            }

            PreferenceSettings.Initialize();

            spriteGammaOverride.Gamma = Camera.Gamma *10.0f;

        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_ESC))
            {
                if (isSettingsOpen)
                {
                    CloseSettings();
                }
            }
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

        public void RestoreDefaults()
        {
            
            PreferenceSettings.RestoreDefaults();

            //spriteGammaOverride.Gamma = Camera.Gamma * 10.0f;

            if (borderAnim != null)
            {
                borderAnim.SyncSlidersToEngine();
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
            isSettingsOpen = true;

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
            isSettingsOpen = false;

            PreferenceSettings.SavePreferences();

            spriteGammaOverride.Gamma = Camera.Gamma * 10.0f;

            if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(false);
                AudioSettings.PlaySFX("PauseTransitionOut");
            }


        }


    }
}

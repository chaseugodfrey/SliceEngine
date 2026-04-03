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
        private SettingsBorderAnimation borderAnim;
        private SettingsBGAnimation bgAnim;

        
        private bool isSettingsOpen = false;

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup_Final");
            bgAnimationObject = FindGameObjectWithName("SettingsBGSpriteSheet");


            
            if (settingsPopup != null)
            {
                borderAnim = settingsPopup.As<SettingsBorderAnimation>();
                if (borderAnim != null)
                {
                    borderAnim.pages[0] = FindGameObjectWithName("AudioSettingsPage");
                    borderAnim.pages[1] = FindGameObjectWithName("GraphicsSettingsPage");
                    
                    borderAnim.coreElements[4] = FindGameObjectWithName("ReturnToTitleButton");

                    borderAnim.titleElements[0] = FindGameObjectWithName("MiniTitleText");
                }
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


            if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(false);
            }

            // Force close everything
            if (settingsPopup != null) settingsPopup.SetActive(false);
            

            SliceLog.Console("Resume");

            Cursor.state = Cursor.STATE.DISABLED;
            Time.timeScale = 1.0f;
        }

        public void OpenSubSettings()
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

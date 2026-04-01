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
        private GameObject settingsPanel;
        private GameObject settingsPopup;
        private SettingsBorderAnimation borderAnim;

        private bool isPauseMenuOpen = false;
        private bool isSubSettingsOpen = false;

        public override void OnCreate()
        {
            settingsPanel = FindGameObjectWithName("GameSettings");
            settingsPopup = FindGameObjectWithName("Settings_Popup");

            if (settingsPanel != null) settingsPanel.SetActive(false);
            if (settingsPopup != null)
            {
                
                borderAnim = settingsPopup.As<SettingsBorderAnimation>();
                settingsPopup.SetActive(false);
            }

            isPauseMenuOpen = false;
            isSubSettingsOpen = false;

            
        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_ESC) && Bootstrap.HUDManager.CheckLoseScreen() == false)
            {
                if (isSubSettingsOpen)
                {
                    CloseSubSettings();
                }
                
                else if (!isPauseMenuOpen)
                {
                    OpenPauseMenu();

                }

            }
        }

        public void OpenPauseMenu()
        {
            if (settingsPanel != null) settingsPanel.SetActive(true);
            isPauseMenuOpen = true;
            Cursor.state = Cursor.STATE.DEFAULT;
            Time.timeScale = 0.0f;
        }


        public void ResumeGame()
        {
            // Force close everything
            if (settingsPanel != null) settingsPanel.SetActive(false);
            if (settingsPopup != null) settingsPopup.SetActive(false);
            isPauseMenuOpen = false;
            isSubSettingsOpen = false;

            SliceLog.Console("Resume");

            Cursor.state = Cursor.STATE.DISABLED;
            Time.timeScale = 1.0f;
        }

        public void OpenSubSettings()
        {

            //if (settingsPanel != null) settingsPanel.SetActive(false);
            //if (settingsPopup != null) settingsPopup.SetActive(true);
            if (borderAnim != null)
            {
                
                
            }
            

            isPauseMenuOpen = false;
            isSubSettingsOpen = true;
        }

        public void CloseSubSettings()
        {
            // Hide Settings, Show Pause Menu
            //if (settingsPopup != null) settingsPopup.SetActive(false);
            //if (settingsPanel != null) settingsPanel.SetActive(true);

            if (borderAnim != null)
            {
                borderAnim.StartSettingsPopupAnimation(false);
            }

            isSubSettingsOpen = false;
            isPauseMenuOpen = true;

            //Bootstrap.HUDManager.CursorChecking(Cursor.state);
        }
    }
}

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

        private bool isPauseMenuOpen = false;
        private bool isSubSettingsOpen = false;

        public override void OnCreate()
        {
            settingsPanel = FindGameObjectWithName("Game Settings");
            settingsPopup = FindGameObjectWithName("Settings_Popup");

            if (settingsPanel != null) settingsPanel.SetActive(false);
            if (settingsPopup != null) settingsPopup.SetActive(false);

            isPauseMenuOpen = false;
            isSubSettingsOpen = false;

            
        }

        public override void OnUpdate(float dt)
        {
            if(Input.IsKeyPressed(Keys.KEY_ESC))
            {
                if (isSubSettingsOpen)
                {
                    CloseSubSettings();
                    CursorChecking(Cursor.state);
                }
                // 2. If nothing is open, open the Pause Menu
                else if (!isPauseMenuOpen)
                {
                    OpenPauseMenu();
                    CursorChecking(Cursor.state);

                }
                // 3. If Pause Menu is open, close it (Resume Game)
                else
                {
                    
                    ResumeGame();
                    CursorChecking(Cursor.state);
                }

            }
        }

        public void OpenPauseMenu()
        {
            if (settingsPanel != null) settingsPanel.SetActive(true);
            isPauseMenuOpen = true;
        }


        public void ResumeGame()
        {
            // Force close everything
            if (settingsPanel != null) settingsPanel.SetActive(false);
            if (settingsPopup != null) settingsPopup.SetActive(false);
            isPauseMenuOpen = false;
            isSubSettingsOpen = false;
        }

        public void OpenSubSettings()
        {
            // Hide Pause Menu, Show Settings
            if (settingsPanel != null) settingsPanel.SetActive(false);
            if (settingsPopup != null) settingsPopup.SetActive(true);

            isPauseMenuOpen = false;
            isSubSettingsOpen = true;
        }

        public void CloseSubSettings()
        {
            // Hide Settings, Show Pause Menu
            if (settingsPopup != null) settingsPopup.SetActive(false);
            if (settingsPanel != null) settingsPanel.SetActive(true);

            isSubSettingsOpen = false;
            isPauseMenuOpen = true;
        }

        public void CursorChecking(Cursor.STATE currentCursorState)
        {
            if(currentCursorState == Cursor.STATE.DISABLED)
            {
                Cursor.state = Cursor.STATE.DEFAULT;
            }
            else
            {
                Cursor.state = Cursor.STATE.DISABLED;
            }
        }
    }
}

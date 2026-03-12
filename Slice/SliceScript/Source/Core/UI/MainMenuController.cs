using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;


namespace SliceEngine
{
    public class MainMenuController : SliceBehaviour
    {

        public float duration = 0.5f;
        private GameObject settingsPopup;
        private GameObject settingsSliders;
        private GameObject closeSettingsButton;
        private GameObject settingsFrontBGPopup;
        private GameObject settingsBackBGPopup;
        private GameObject settingsFrontTitleBGPopup;
        private GameObject settingsBackTitleBGPopup;
        private GameObject backTitleObj;
        private GameObject frontTitleObj;

        private RectTransform settingsFrontBgTrans;
        private RectTransform settingsBackBgTrans;
        private RectTransform settingsFrontTitleBgTrans;
        private RectTransform settingsBackTitleBgTrans;

        private FontRenderer frontText;
        private FontRenderer backText;

        public bool useGlitch = true;

        private GameObject MainMenuCanvas;
        private bool openSettings = false;

        public float defaultHeight = 0f;
        public float finalHeight = 0f;
        public int backBGFinalXPos = 0;
        public int backBGFinalYPos = 0;
        public int defaultFrontTitleBGWidth = 0;
        public int defaultBackTitleBGWidth = 0;
        public int finalBackTitleBGWidth = 0;
        public int finalFrontTitleBGWidth = 0;

        private float animationTimer = 0f;

        public string textToShow = "";
        private UIBorderAnimation uiBorderAnimController;

        public override void OnCreate()
        {
            settingsPopup = FindGameObjectWithName("Settings_Popup");
            settingsFrontBGPopup = FindGameObjectWithName("PopupBG");
            settingsBackBGPopup = FindGameObjectWithName("PopupBG_1");
            settingsFrontTitleBGPopup = FindGameObjectWithName("TitleTextFrontBG");
            settingsBackTitleBGPopup = FindGameObjectWithName("TitleTextBackBG");
            settingsSliders = FindGameObjectWithName("SettingsSlider");
            closeSettingsButton = FindGameObjectWithName("CloseSettings");
            MainMenuCanvas = FindGameObjectWithName("MainMenu_Canvas");
            frontTitleObj = FindGameObjectWithName("FrontTitleText");
            backTitleObj = FindGameObjectWithName("BackTitleText");

            if (settingsPopup != null)
            {
                settingsPopup.SetActive(false);
                settingsFrontBgTrans = settingsFrontBGPopup.GetComponent<RectTransform>();
                settingsBackBgTrans = settingsBackBGPopup.GetComponent<RectTransform>();
                settingsFrontTitleBgTrans = settingsFrontTitleBGPopup.GetComponent<RectTransform>();
                settingsBackTitleBgTrans = settingsBackTitleBGPopup.GetComponent<RectTransform>();

            }

            if(frontTitleObj != null)
            {
                frontText = frontTitleObj.GetComponent<FontRenderer>();
            }

            if (backTitleObj != null)
            {
                backText = backTitleObj.GetComponent<FontRenderer>();
            }


        }

        public override void OnUpdate(float dt)
        {
            if (openSettings || animationTimer > 0f)
            {
                // 1. Directional Timer Logic
                if (openSettings)
                    animationTimer += Time.deltaTime / duration;
                else
                    animationTimer -= Time.deltaTime / duration;

                animationTimer = Utilities.Clamp(animationTimer, 0f, 1f);

                // 2. STAGE 1: Background & Title BG (0.0 to 0.5 range)
                // Use InverseLerp to create a progress factor specifically for this window
                float bgProgress = Utilities.InverseLerp(0f, 0.5f, animationTimer);

                if (settingsFrontBgTrans != null)
                {
                    float currentVal = Utilities.SmoothStep(defaultHeight, finalHeight, bgProgress);
                    float currentFrontTitleBGWidth = Utilities.SmoothStep(defaultFrontTitleBGWidth, finalFrontTitleBGWidth, bgProgress);
                    float currentBackTitleBGWidth = Utilities.SmoothStep(defaultBackTitleBGWidth, finalBackTitleBGWidth, bgProgress);

                    settingsFrontBgTrans.Height = (int)currentVal;
                    settingsBackBgTrans.Height = (int)currentVal;
                    settingsFrontTitleBgTrans.Right = (int)currentFrontTitleBGWidth;
                    settingsBackTitleBgTrans.Right = (int)currentBackTitleBGWidth;

                    // Subtle position offset for back BG if needed
                    if (bgProgress >= 1f && settingsBackBgTrans.Pos_X != backBGFinalXPos)
                    {
                        settingsBackBgTrans.Pos_X = (int)Utilities.SmoothStep(settingsBackBgTrans.Pos_X, backBGFinalXPos, 0.5f);
                        settingsBackBgTrans.Pos_Y = (int)Utilities.SmoothStep(settingsBackBgTrans.Pos_Y, backBGFinalYPos, 0.5f);
                    }
                }

                // 3. STAGE 2: Title Text Typewriter (0.5 to 1.0 range)
                // textProgress will stay at 0 until animationTimer exceeds 0.5
                float textProgress = Utilities.InverseLerp(0.5f, 1.0f, animationTimer);

                if (frontText != null && backText != null && !string.IsNullOrEmpty(textToShow))
                {
                    int charactersToShow = (int)(textProgress * textToShow.Length);
                    string currentStr = textToShow.Substring(0, charactersToShow);

                    frontText.Text_val = currentStr;
                    backText.Text_val = currentStr;
                }

                // 4. UI Element Visibility (Only at full completion)
                if (animationTimer >= 1.0f && openSettings)
                {
                    if (settingsSliders != null) settingsSliders.SetActive(true);
                    if (closeSettingsButton != null) closeSettingsButton.SetActive(true);
                }
                else
                {
                    // Hide immediately if we aren't at the end of the sequence
                    if (settingsSliders != null) settingsSliders.SetActive(false);
                    if (closeSettingsButton != null) closeSettingsButton.SetActive(false);
                }

                // 5. Deactivation Logic
                if (animationTimer <= 0f && !openSettings)
                {
                    if (settingsPopup != null) settingsPopup.SetActive(false);
                    if (MainMenuCanvas != null) MainMenuCanvas.SetActive(true);
                }
            }
        }

        public void StartGame(string sceneName)
        {
            if (sceneName != "")
            {
                SceneManager.LoadScene(sceneName);
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

            if (settingsPopup != null)
            {
                settingsPopup.SetActive(true);
                openSettings = true;
                
            }

            if (MainMenuCanvas != null)
            {
                MainMenuCanvas.SetActive(false);
            }
        }

        public void CloseSettings()
        {
            //if (settingsPopup != null) settingsPopup.SetActive(false);
            //if(MainMenuCanvas != null) MainMenuCanvas.SetActive(true);
            openSettings = false;
        }


    }
}
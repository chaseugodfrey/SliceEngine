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
            settingsSliders = FindGameObjectWithName("SettingsSliders");
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
            if(openSettings)
            {

                animationTimer += Time.deltaTime / duration;

                animationTimer = Utilities.Clamp(animationTimer, 0f, 1f);

                //Console.WriteLine($"{animationTimer}");

                if(settingsFrontBgTrans != null)
                {
                    float currentVal = Utilities.SmoothStep(defaultHeight, finalHeight, animationTimer);
                    float currentFrontTitleBGWidth = Utilities.SmoothStep(defaultFrontTitleBGWidth, finalFrontTitleBGWidth, animationTimer);
                    float currentBackTitleBGWidth = Utilities.SmoothStep(defaultBackTitleBGWidth, finalBackTitleBGWidth, animationTimer);
                    settingsFrontBgTrans.Height = (int)currentVal;
                    settingsBackBgTrans.Height = (int)currentVal;
                    settingsFrontTitleBgTrans.Right = (int)currentFrontTitleBGWidth;
                    settingsBackTitleBgTrans.Right = (int)currentBackTitleBGWidth;

                    //Console.WriteLine($"{settingsBgTrans.Height}");
                    if (settingsBackBgTrans.Height == finalHeight && settingsBackBgTrans.Pos_X != backBGFinalXPos)
                    {
                        float currentPosX = Utilities.SmoothStep(settingsBackBgTrans.Pos_X, backBGFinalXPos, 0.5f);
                        float currentPosY = Utilities.SmoothStep(settingsBackBgTrans.Pos_Y, backBGFinalYPos, 0.5f);

                        settingsBackBgTrans.Pos_X = (int)currentPosX;
                        settingsBackBgTrans.Pos_Y = (int)currentPosY;
                    }
                }

                if (frontText != null && !string.IsNullOrEmpty(textToShow) && backText != null)
                {
                    int charactersToShow = (int)(animationTimer * textToShow.Length);
                    string displayStr = textToShow.Substring(0, charactersToShow);

                    // Add a random character at the end during reveal
                    if (useGlitch && charactersToShow > 0 && charactersToShow < textToShow.Length)
                    {
                        string glitchChars = "X/#_01";
                        int charIndex = (int)(SliceRandom.ValueFloat() * glitchChars.Length);
                        displayStr += glitchChars[charIndex];
                    }

                    frontText.Text_val = displayStr;
                    backText.Text_val = displayStr;
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
            if (settingsPopup != null) settingsPopup.SetActive(false);
            if(MainMenuCanvas != null) MainMenuCanvas.SetActive(true);
        }


    }
}
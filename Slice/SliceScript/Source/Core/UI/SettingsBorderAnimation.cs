using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SettingsBorderAnimation : SliceBehaviour
    {
        public float duration = 0.5f;
        public GameObject settingsFrontBG;
        public GameObject settingsBackBG;
        public GameObject settingsFrontTitleBG;
        public GameObject settingsBackTitleBG;
        public GameObject frontTitleObj;
        public GameObject backTitleObj;
        public GameObject settingsSliders;
        public GameObject closeSettingsButton;
        public GameObject menuCanvasObj;

        private RectTransform frontBgTrans, backBgTrans, frontTitleBgTrans, backTitleBgTrans;
        private FontRenderer frontText, backText;

        public int defaultHeight = 0;
        public int finalHeight = 0;
        public int defaultFrontTitleBGWidth = 0;
        public int finalFrontTitleBGWidth = 0;
        public int defaultBackTitleBGWidth = 0;
        public int finalBackTitleBGWidth = 0;
        public string textToShow = "";

        private float animationTimer = 0f;
        private bool isOpening = false;
        private bool isActive = false;
        public bool useGlitch = true;

        public override void OnCreate()
        {
            if (settingsFrontBG != null) frontBgTrans = settingsFrontBG.GetComponent<RectTransform>();
            if (settingsBackBG != null) backBgTrans = settingsBackBG.GetComponent<RectTransform>();
            if (settingsFrontTitleBG != null) frontTitleBgTrans = settingsFrontTitleBG.GetComponent<RectTransform>();
            if (settingsBackTitleBG != null) backTitleBgTrans = settingsBackTitleBG.GetComponent<RectTransform>();

            if (frontTitleObj != null) frontText = frontTitleObj.GetComponent<FontRenderer>();
            if (backTitleObj != null) backText = backTitleObj.GetComponent<FontRenderer>();

            // Ensure UI elements start hidden
            if (settingsSliders != null) settingsSliders.SetActive(false);
            if (closeSettingsButton != null) closeSettingsButton.SetActive(false);
        }

        public override void OnUpdate(float dt)
        {
            // 1. Run logic if we are opening or still in the middle of a closing animation
            if (isOpening || animationTimer > 0f)
            {
                // Directional Timer Logic
                if (isOpening)
                    animationTimer += Time.deltaTimeUnscaled / duration;
                else
                    animationTimer -= Time.deltaTimeUnscaled / duration;

                animationTimer = Utilities.Clamp(animationTimer, 0f, 1f);

                // 2. STAGE 1: Background & Title BG (0.0 to 0.5 range)
                // InverseLerp maps the first half of the timer to a 0-1 progress factor
                float bgProgress = Utilities.InverseLerp(0f, 0.5f, animationTimer);

                if (frontBgTrans != null)
                {
                    // Smoothly interpolate heights and widths
                    float currentHeight = Utilities.SmoothStep(defaultHeight, finalHeight, bgProgress);
                    float currentFrontTitleWidth = Utilities.SmoothStep(defaultFrontTitleBGWidth, finalFrontTitleBGWidth, bgProgress);
                    float currentBackTitleWidth = Utilities.SmoothStep(defaultBackTitleBGWidth, finalBackTitleBGWidth, bgProgress);

                    // Apply values to cached RectTransforms
                    frontBgTrans.Height = (int)currentHeight;
                    if (backBgTrans != null) backBgTrans.Height = (int)currentHeight;

                    if (frontTitleBgTrans != null) frontTitleBgTrans.Right = (int)currentFrontTitleWidth;
                    if (backTitleBgTrans != null) backTitleBgTrans.Right = (int)currentBackTitleWidth;
                }

                // 3. STAGE 2: Title Text Typewriter (0.5 to 1.0 range)
                // This factor stays at 0 until the background is 50% done
                float textProgress = Utilities.InverseLerp(0.5f, 1.0f, animationTimer);

                if (frontText != null && !string.IsNullOrEmpty(textToShow))
                {
                    int charactersToShow = (int)(textProgress * textToShow.Length);
                    string currentStr = textToShow.Substring(0, charactersToShow);

                    frontText.Text_val = currentStr;
                    if (backText != null) backText.Text_val = currentStr;
                }

                // 4. UI Element Visibility
                // Show sliders/button only when fully open; hide them immediately when closing starts
                bool isFullyOpen = animationTimer >= 1.0f && isOpening;
                if (settingsSliders != null) settingsSliders.SetActive(isFullyOpen);
                if (closeSettingsButton != null) closeSettingsButton.SetActive(isFullyOpen);

                // 5. Final Deactivation
                if (animationTimer <= 0f && !isOpening)
                {
                    isActive = false;
                    if (menuCanvasObj != null)
                    {
                        menuCanvasObj.SetActive(true);
                    }

                    this.gameObject.SetActive(false);

                }
            }
        }

        public void StartAnimation(bool opening)
        {
            isOpening = opening;
            isActive = true;
            this.gameObject.SetActive(true);
        }

    }
}

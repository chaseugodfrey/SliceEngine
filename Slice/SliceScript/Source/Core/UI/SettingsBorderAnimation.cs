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

            if (settingsSliders != null) settingsSliders.SetActive(false);
            if (closeSettingsButton != null) closeSettingsButton.SetActive(false);
        }

        public override void OnUpdate(float dt)
        {
            
            if (isOpening || animationTimer > 0f)
            {
                // Directional Timer Logic
                if (isOpening)
                {

                    animationTimer += Time.deltaTimeUnscaled / duration;
                }
                else
                {
                    animationTimer -= Time.deltaTimeUnscaled / duration;

                }

                animationTimer = Utilities.Clamp(animationTimer, 0f, 1f);

                float bgProgress = Utilities.InverseLerp(0f, 0.5f, animationTimer);

                if (frontBgTrans != null)
                {
                    
                    float currentHeight = Utilities.SmoothStep(defaultHeight, finalHeight, bgProgress);
                    
                    frontBgTrans.Height = (int)currentHeight;

                }

                
                float textProgress = Utilities.InverseLerp(0.5f, 1.0f, animationTimer);

                if (frontText != null && !string.IsNullOrEmpty(textToShow))
                {
                    int charactersToShow = (int)(textProgress * textToShow.Length);
                    string currentStr = textToShow.Substring(0, charactersToShow);

                    frontText.Text_val = currentStr;
                    if (backText != null) backText.Text_val = currentStr;
                }

                
                bool isFullyOpen = animationTimer >= 1.0f && isOpening;
                if (settingsSliders != null) settingsSliders.SetActive(isFullyOpen);
                if (closeSettingsButton != null) closeSettingsButton.SetActive(isFullyOpen);

                
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

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
        public GameObject menuCanvasObj;
        public GameObject settingsContent;
        public GameObject flickerImage;
        public GameObject systemsTitle;

        public GameObject audioButton;
        public GameObject graphicsButton;

        public GameObject audioSettingsPage;
        public GameObject graphicsSettingsPage;

        public GameObject audioActive;
        public GameObject graphicsActive;
        public GameObject returnToTitleButton;

        public GameObject miniTitleObj;
        public List<GameObject> audioSliders = new List<GameObject>() { null };
        public GameObject gammaSlider;

        private SpriteRenderer audioSprite;
        private FontRenderer audioFont;

        private RectTransform systemsPausedTrans;
        private RectTransform miniTitleTrans;
        private FontRenderer miniTitleText;
        private RectTransform[] audioSliderTrans;

        private SpriteRenderer graphicsSprite;
        private SpriteRenderer flickerImageSprite;
        private FontRenderer graphicsFont;

        private RectTransform frontBgTrans;

        private Vector4 highlightFontColor = new Vector4(68f / 255f, 51f / 255f, 20f / 255f, 1f);

        private Vector4 defaultFontColor = new Vector4(1f, 1f, 1f, 1f);

        public int defaultHeight = 0;
        public int finalHeight = 0;

        private float animationTimer = 0f;
        private bool isOpening = false;
        private bool isActive = false;

        private bool isAudioPageSelected = true;

        public float flickerSpeed = 0.05f;
        public float flickerTotalDuration = 0.5f;
        private float flickerTimer = 0f;

        public override void OnCreate()
        {
            if (settingsFrontBG != null) frontBgTrans = settingsFrontBG.GetComponent<RectTransform>();

            if (flickerImage != null)
            {
                flickerImageSprite = flickerImage.GetComponent<SpriteRenderer>();
            }

            if (miniTitleObj != null)
            {
                miniTitleText = miniTitleObj.GetComponent<FontRenderer>();
                miniTitleTrans = miniTitleObj.GetComponent<RectTransform>();
            }


            if (audioActive != null)
            {
                audioSprite = audioActive.GetComponent<SpriteRenderer>();

            }

            if (graphicsActive != null)
            {
                graphicsSprite = graphicsActive.GetComponent<SpriteRenderer>();

            }

            if (systemsTitle != null)
            {
                systemsPausedTrans = systemsTitle.GetComponent<RectTransform>();
            }

            if (audioButton != null)
            {
                audioFont = audioButton.GetComponent<FontRenderer>();
            }

            if (graphicsButton != null)
            {
                graphicsFont = graphicsButton.GetComponent<FontRenderer>();
            }

            if(audioSliders.Count > 0)
            {
                audioSliderTrans = new RectTransform[audioSliders.Count];

                for(int i = 0; i < audioSliders.Count; i++)
                {
                    if (audioSliders[i] != null)
                    {
                        audioSliderTrans[i] = audioSliders[i].GetComponent<RectTransform>();
                    }
                }
            }
        }



        public override void OnUpdate(float dt)
        {
            if (flickerImageSprite != null)
            {
                flickerTimer += Time.deltaTimeUnscaled;
                Vector4 color = flickerImageSprite.Colour;

                if (isOpening)
                {
                    if (flickerTimer < flickerTotalDuration)
                    {
                        float t = Utilities.PingPong(flickerTimer, flickerSpeed) / flickerSpeed;
                        color.w = Utilities.Lerp(1.0f, 0.0f, t);
                    }
                    else
                    {
                        color.w = 0.0f;
                    }
                }
                else
                {
                    if (flickerTimer < flickerTotalDuration)
                    {
                        float t = Utilities.PingPong(flickerTimer, flickerSpeed) / flickerSpeed;
                        color.w = Utilities.Lerp(0.0f, 1.0f, t);
                    }
                    else
                    {
                        color.w = 1.0f;
                    }
                }

                flickerImageSprite.Colour = color;
            }

            if (isOpening || animationTimer > 0f)
            {
                if (isOpening)
                {
                    animationTimer += Time.deltaTimeUnscaled / duration;
                }
                else
                {
                    animationTimer -= Time.deltaTimeUnscaled / duration;
                }

                animationTimer = Utilities.Clamp(animationTimer, 0f, 0.5f);
                float bgProgress = Utilities.InverseLerp(0f, 0.3f, animationTimer);

                if (frontBgTrans != null)
                {
                    float currentHeight = Utilities.SmoothStep(defaultHeight, finalHeight, bgProgress);
                    float currentWidth = Utilities.SmoothStep(1500, -572, bgProgress);
                    
                    float currentSliderWidth = Utilities.SmoothStep(0, -1161, bgProgress);
                    float currentMiniTitleWidth = Utilities.SmoothStep(800, -302, bgProgress);
                    frontBgTrans.Height = (int)currentHeight;
                    systemsPausedTrans.Right = (int)currentWidth;
                    
                    miniTitleTrans.Right = (int)currentMiniTitleWidth;

                    for(int i = 0; i < audioSliders.Count; i++)
                    {
                        audioSliderTrans[i].Right = (int)currentSliderWidth;
                    }

                }

                bool isFullyOpen = true;

                // Manage visibility of elements
                if (returnToTitleButton != null) returnToTitleButton.SetActive(isFullyOpen);

                if (isFullyOpen)
                {
                    if (settingsContent != null) settingsContent.SetActive(true);
                    if (audioSettingsPage != null) audioSettingsPage.SetActive(isAudioPageSelected);
                    if (graphicsSettingsPage != null) graphicsSettingsPage.SetActive(!isAudioPageSelected);
                    UpdateTitleText();
                }
                else
                {

                    //if (audioSettingsPage != null) audioSettingsPage.SetActive(false);
                    //if (graphicsSettingsPage != null) graphicsSettingsPage.SetActive(false);
                    if (settingsContent != null) settingsContent.SetActive(false);
                }

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

        private void UpdateButtonVisuals()
        {
            // Update Audio Button
            SliceLog.Log("isAudioPageSelected : " + isAudioPageSelected);
            

            if (audioSprite != null)
            {
                audioSprite.SetEnabled(isAudioPageSelected);

            }
            if (audioFont != null) audioFont.Colour = isAudioPageSelected ? highlightFontColor : defaultFontColor;

            if (graphicsSprite != null)
            {
                graphicsSprite.SetEnabled(!isAudioPageSelected);
            }
            if (graphicsFont != null) graphicsFont.Colour = !isAudioPageSelected ? highlightFontColor : defaultFontColor;
        }

        private void UpdateTitleText()
        {
            if (miniTitleText != null)
            {
                miniTitleText.Text_val = isAudioPageSelected ? "ADJUST AUDIO SETTINGS" : "ADJUST PREFERRED BRIGHTNESS";
            }
        }

        public void StartSettingsPopupAnimation(bool opening)
        {
            this.gameObject.SetActive(true);
            isOpening = opening;
            flickerTimer = 0f; // Reset flicker timer
            UpdateButtonVisuals();
            isActive = true;


            if (!opening)
            {
                if (settingsContent != null) settingsContent.SetActive(false);
            }
        }

        public void SwitchToPage(bool isAudio)
        {
            isAudioPageSelected = isAudio;

            // Apply immediately if already open
            if (animationTimer >= 1.0f && isOpening)
            {
                if (audioSettingsPage != null) audioSettingsPage.SetActive(isAudio);
                if (graphicsSettingsPage != null) graphicsSettingsPage.SetActive(!isAudio);
                UpdateTitleText();
            }

            UpdateButtonVisuals();
        }
    }
}

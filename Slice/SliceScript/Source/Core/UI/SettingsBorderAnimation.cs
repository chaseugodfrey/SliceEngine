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
        public GameObject settingsContent;
        public GameObject flickerImage;
        public GameObject systemsTitle;

        public GameObject audioSettingsPage;
        public GameObject graphicsSettingsPage;

        public GameObject audioActive;
        public GameObject audioLabel;
        public GameObject graphicsActive;
        public GameObject graphicsLabel;
        public GameObject returnToTitleButton;

        public GameObject miniTitleTextObj;

        private SpriteRenderer audioSprite;
        private FontRenderer audioFont;

        private RectTransform systemsPausedTrans;

        private SpriteRenderer graphicsSprite;
        private SpriteRenderer flickerImageSprite;
        private FontRenderer graphicsFont;

        private RectTransform frontBgTrans;
        private FontRenderer miniTitleText;

        private Vector4 highlightSpriteColor = new Vector4(151f / 255f, 63f / 255f, 0f, 0f);
        private Vector4 highlightFontColor = new Vector4(68f / 255f, 51f / 255f, 20f / 255f, 1f);


        private Vector4 defaultSpriteColor = new Vector4(1f, 1f, 1f, 1f);
        private Vector4 defaultFontColor = new Vector4(1f, 1f, 1f, 1f);

        public int defaultHeight = 0;
        public int finalHeight = 0;
        
        private float animationTimer = 0f;
        private bool isOpening = false;
        private bool isActive = false;
        public bool useGlitch = true;

        private bool isAudioPageSelected = true;

        public float flickerSpeed = 0.05f;
        public float flickerTotalDuration = 0.5f;
        private float flickerTimer = 0f;

        public override void OnCreate()
        {
            if (settingsFrontBG != null) frontBgTrans = settingsFrontBG.GetComponent<RectTransform>();
            if (settingsSliders != null) settingsSliders.SetActive(false);

            if(flickerImage != null)
            {
                flickerImageSprite = flickerImage.GetComponent<SpriteRenderer>();
            }

            if (miniTitleTextObj != null)
            {
                miniTitleText = miniTitleTextObj.GetComponent<FontRenderer>();
            }

            
            if (audioActive != null)
            {   
                audioSprite = audioActive.GetComponent<SpriteRenderer>();
                
            }

            if (graphicsActive != null)
            {
                graphicsSprite = graphicsActive.GetComponent<SpriteRenderer>();
                
            }

            if(systemsTitle != null)
            {
                systemsPausedTrans = systemsTitle.GetComponent<RectTransform>();
            }

            if (audioLabel != null)
            {
                audioFont = audioLabel.GetComponent<FontRenderer>();
            }

            if (graphicsLabel != null)
            {
                graphicsFont = graphicsLabel.GetComponent<FontRenderer>();
            }
        }

      

        public override void OnUpdate(float dt)
        {
            if (flickerImageSprite != null)
            {
                flickerTimer += Time.deltaTimeUnscaled;
                Vector4 color = flickerImageSprite.Colour;

                if (flickerTimer < flickerTotalDuration)
                {
                    float t = Utilities.PingPong(flickerTimer, flickerSpeed) / flickerSpeed;
                    color.w = Utilities.Lerp(0.5f, 0.0f, t);
                }
                else
                {
                    color.w = 0.0f;
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

            animationTimer = Utilities.Clamp(animationTimer, 0f, 1f);
            float bgProgress = Utilities.InverseLerp(0f, 0.5f, animationTimer);

            if (frontBgTrans != null)
            {
                float currentHeight = Utilities.SmoothStep(defaultHeight, finalHeight, bgProgress);
                float currentWidth = Utilities.SmoothStep(0, 572, bgProgress);
                frontBgTrans.Height = (int)currentHeight;
                systemsPausedTrans.Width = (int)currentWidth *(-1);
                
            }

            bool isFullyOpen = animationTimer >= 1.0f && isOpening;

            // Manage visibility of elements
            if (settingsSliders != null) settingsSliders.SetActive(isFullyOpen);
            if (closeSettingsButton != null) closeSettingsButton.SetActive(isFullyOpen);
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
        //if (audioBtnComp != null) audioBtnComp.SetEnabled(!isAudioPageSelected);
        //if (audioSprite != null) audioSprite.Colour = isAudioPageSelected ? highlightSpriteColor : defaultSpriteColor;
        //if (audioFont != null) audioFont.Colour = isAudioPageSelected ? highlightFontColor : defaultFontColor;

        // Update Graphics Button
        //if (graphicsBtnComp != null) graphicsBtnComp.SetEnabled(isAudioPageSelected);
        //if (graphicsSprite != null) graphicsSprite.Colour = !isAudioPageSelected ? highlightSpriteColor : defaultSpriteColor;
        //if (graphicsFont != null) graphicsFont.Colour = !isAudioPageSelected ? highlightFontColor : defaultFontColor;

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

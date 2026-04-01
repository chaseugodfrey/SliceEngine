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
        public GameObject settingsBGAnim;
        public GameObject settingsBG;
        public GameObject settingsContent;

        public GameObject audioSettingsPage;
        public GameObject graphicsSettingsPage;

        public GameObject audioButton;
        public GameObject graphicsButton;
        public GameObject returnToTitleButton;

        public GameObject miniTitleTextObj;
        public GameObject miniTitleObject;

        private Button audioBtnComp;
        private SpriteRenderer audioSprite;
        private FontRenderer audioFont;

        private Button graphicsBtnComp;
        private SpriteRenderer graphicsSprite;
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

        public override void OnCreate()
        {
            if (settingsFrontBG != null) frontBgTrans = settingsFrontBG.GetComponent<RectTransform>();
            if (settingsSliders != null) settingsSliders.SetActive(false);

            if (miniTitleTextObj != null)
            {
                miniTitleText = miniTitleTextObj.GetComponent<FontRenderer>();
            }

            // --- New logic: Fetch components ---
            if (audioButton != null)
            {
                audioBtnComp = audioButton.GetComponent<Button>();
                audioSprite = audioButton.GetComponent<SpriteRenderer>();
                // Assuming the FontRenderer is on the button itself or you can fetch it from a child
                audioFont = audioButton.GetComponent<FontRenderer>();
            }

            if (graphicsButton != null)
            {
                graphicsBtnComp = graphicsButton.GetComponent<Button>();
                graphicsSprite = graphicsButton.GetComponent<SpriteRenderer>();
                graphicsFont = graphicsButton.GetComponent<FontRenderer>();
            }
        }

      

        public override void OnUpdate(float dt)
        {
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
                    frontBgTrans.Height = (int)currentHeight;
                }

                bool isFullyOpen = animationTimer >= 1.0f && isOpening;

                // Manage visibility of elements
                if (miniTitleObject != null) miniTitleObject.SetActive(isFullyOpen);
                if (settingsSliders != null) settingsSliders.SetActive(isFullyOpen);
                if (closeSettingsButton != null) closeSettingsButton.SetActive(isFullyOpen);
                if (audioButton != null) audioButton.SetActive(isFullyOpen);
                if (graphicsButton != null) graphicsButton.SetActive(isFullyOpen);
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
            if (audioBtnComp != null) audioBtnComp.SetEnabled(!isAudioPageSelected);
            if (audioSprite != null) audioSprite.Colour = isAudioPageSelected ? highlightSpriteColor : defaultSpriteColor;
            if (audioFont != null) audioFont.Colour = isAudioPageSelected ? highlightFontColor : defaultFontColor;

            // Update Graphics Button
            if (graphicsBtnComp != null) graphicsBtnComp.SetEnabled(isAudioPageSelected);
            if (graphicsSprite != null) graphicsSprite.Colour = !isAudioPageSelected ? highlightSpriteColor : defaultSpriteColor;
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
            isOpening = opening;
            UpdateButtonVisuals();
            isActive = true;
            this.gameObject.SetActive(true);
            
            // If closing, ensure everything is hidden immediately or starts hiding
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

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

        //[Header("0: FrontBG, 1: MenuCanvas, 2: SettingsContent, 3: FlickerImage, 4: ReturnToTitle")]
        public List<GameObject> coreElements = new List<GameObject>() { null, null, null, null, null };

        //[Header("0: Master, 1: BGM, 2: SFX, 3: Gamma")]
        public List<GameObject> sliders = new List<GameObject>() { null, null, null, null };

        //[Header("0: Master, 1: BGM, 2: SFX, 3: Gamma, 4: Restore")]
        public List<GameObject> sliderLabelElements = new List<GameObject>() { null, null, null, null, null };

        //[Header("0: AudioPage, 1: GraphicsPage")]
        public List<GameObject> pages = new List<GameObject>() { null, null };

        //[Header("0: AudioActive, 1: AudioLabel, 2: GraphicsActive, 3: GraphicsLabel")]
        public List<GameObject> tabElements = new List<GameObject>() { null, null, null, null };

        //[Header("0: MiniTitleText, 1: MiniTitleBG, 2: SystemsTitle")]
        public List<GameObject> titleElements = new List<GameObject>() { null, null, null };

        //[Header("0: EscBtn, 1: EscText")]
        public List<GameObject> escElements = new List<GameObject>() { null, null };

        private SpriteRenderer audioSprite;

        private RectTransform systemsPausedTrans;
        private RectTransform[] sliderTrans;
        private RectTransform frontBgTrans;
        private RectTransform miniTitleBGTrans;
        private RectTransform escTrans;

        private SpriteRenderer graphicsSprite;
        private SpriteRenderer flickerImageSprite;

        private FontRenderer audioFont;
        private FontRenderer graphicsFont;
        private FontRenderer escFont;

        private FontRenderer masterLabel;
        private FontRenderer bgmLabel;
        private FontRenderer sfxLabel;
        private FontRenderer gammaLabel;
        private FontRenderer restoreLabel;

        private FontRenderer miniTitleText;

        
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

        private float typewriterSpeed = 30f;
        private float audioTypewriterProgress = 0f;
        private float graphicsTypewriterProgress = 0f;
        private float miniTitleTypewriterProgress = 0f;
        private float escTypewriterProgress = 0f;

        private float masterProgress = 0f;
        private float bgmProgress = 0f;
        private float sfxProgress = 0f;
        private float gammaProgress = 0f;
        private float restoreProgress = 0f;


        private string audioFullText = "";
        private string graphicsFullText = "";
        private string miniTitleFullText = "";
        private string escFullText = "";

        private string masterFullText = "";
        private string bgmFullText = "";
        private string sfxFullText = "";
        private string gammaFullText = "";
        private string restoreFullText = "";


        public override void OnCreate()
        {
            if (coreElements.Count > 0 && coreElements[0] != null) frontBgTrans = coreElements[0].GetComponent<RectTransform>();
            

            if(coreElements.Count > 3 && coreElements[3] != null)
            {
                flickerImageSprite = coreElements[3].GetComponent<SpriteRenderer>();
            }

            if (titleElements.Count > 0 && titleElements[0] != null)
            {
                miniTitleText = titleElements[0].GetComponent<FontRenderer>();
            }

            
            if (tabElements.Count > 0 && tabElements[0] != null)
            {   
                audioSprite = tabElements[0].GetComponent<SpriteRenderer>();
                
            }

            if (tabElements.Count > 2 && tabElements[2] != null)
            {
                graphicsSprite = tabElements[2].GetComponent<SpriteRenderer>();
                
            }

            if(titleElements.Count > 2 && titleElements[2] != null)
            {
                systemsPausedTrans = titleElements[2].GetComponent<RectTransform>();
            }

            if (tabElements.Count > 1 && tabElements[1] != null)
            {
                audioFont = tabElements[1].GetComponent<FontRenderer>();
                audioFullText = audioFont.Text_val;
            }

            if (tabElements.Count > 3 && tabElements[3] != null)
            {
                graphicsFont = tabElements[3].GetComponent<FontRenderer>();
                graphicsFullText = graphicsFont.Text_val;
            }

            if(sliders.Count > 0)
            {
                sliderTrans = new RectTransform[sliders.Count];
                for (int i = 0; i < sliders.Count; i++)
                {
                    if (sliders[i] != null) sliderTrans[i] = sliders[i].GetComponent<RectTransform>();
                }
            }

            if (sliderLabelElements.Count >= 5)
            {
                if (sliderLabelElements[0] != null) { masterLabel = sliderLabelElements[0].GetComponent<FontRenderer>(); if (masterLabel != null) masterFullText = masterLabel.Text_val; }
                if (sliderLabelElements[1] != null) { bgmLabel = sliderLabelElements[1].GetComponent<FontRenderer>(); if (bgmLabel != null) bgmFullText = bgmLabel.Text_val; }
                if (sliderLabelElements[2] != null) { sfxLabel = sliderLabelElements[2].GetComponent<FontRenderer>(); if (sfxLabel != null) sfxFullText = sfxLabel.Text_val; }
                if (sliderLabelElements[3] != null) { gammaLabel = sliderLabelElements[3].GetComponent<FontRenderer>(); if (gammaLabel != null) gammaFullText = gammaLabel.Text_val; }
                if (sliderLabelElements[4] != null) { restoreLabel = sliderLabelElements[4].GetComponent<FontRenderer>(); if (restoreLabel != null) restoreFullText = restoreLabel.Text_val; }
            }

            

            if(titleElements.Count > 1 && titleElements[1] != null)
            {
                miniTitleBGTrans = titleElements[1].GetComponent<RectTransform>();
            }

            if(escElements.Count > 0 && escElements[0] != null)
            {
                escTrans = escElements[0].GetComponent<RectTransform>();
            }

            if(escElements.Count > 1 && escElements[1] != null)
            {
                escFont = escElements[1].GetComponent<FontRenderer>();
                escFullText = escFont.Text_val;
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
                        color.w = Utilities.Lerp(0.5f, 0.0f, t);
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
                        color.w = Utilities.Lerp(0.0f, 0.5f, t);
                    }
                    else
                    {
                        color.w = 0.5f;
                    }
                }

                flickerImageSprite.Colour = color;
            }

            // Typewriter Updates
            if (isActive)
            {
                if (isOpening)
                {
                    if (audioFont != null && audioTypewriterProgress < audioFullText.Length)
                    {
                        audioTypewriterProgress += Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Min((float)audioFullText.Length, audioTypewriterProgress);
                        audioFont.Text_val = audioFullText.Substring(0, length);
                    }

                    if (graphicsFont != null && graphicsTypewriterProgress < graphicsFullText.Length)
                    {
                        graphicsTypewriterProgress += Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Min((float)graphicsFullText.Length, graphicsTypewriterProgress);
                        graphicsFont.Text_val = graphicsFullText.Substring(0, length);
                    }

                    if (escFont != null && escTypewriterProgress < escFullText.Length)
                    {
                        escTypewriterProgress += Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Min((float)escFullText.Length, escTypewriterProgress);
                        escFont.Text_val = escFullText.Substring(0, length);
                    }

                    if (miniTitleText != null && miniTitleTypewriterProgress < miniTitleFullText.Length)
                    {
                        miniTitleTypewriterProgress += Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Min((float)miniTitleFullText.Length, miniTitleTypewriterProgress);
                        miniTitleText.Text_val = miniTitleFullText.Substring(0, length);
                    }

                    // Slider Labels Opening
                    if (masterLabel != null && masterProgress < masterFullText.Length) { masterProgress += Time.deltaTimeUnscaled * typewriterSpeed; masterLabel.Text_val = masterFullText.Substring(0, (int)Utilities.Min(masterFullText.Length, masterProgress)); }
                    if (bgmLabel != null && bgmProgress < bgmFullText.Length) { bgmProgress += Time.deltaTimeUnscaled * typewriterSpeed; bgmLabel.Text_val = bgmFullText.Substring(0, (int)Utilities.Min(bgmFullText.Length, bgmProgress)); }
                    if (sfxLabel != null && sfxProgress < sfxFullText.Length) { sfxProgress += Time.deltaTimeUnscaled * typewriterSpeed; sfxLabel.Text_val = sfxFullText.Substring(0, (int)Utilities.Min(sfxFullText.Length, sfxProgress)); }
                    if (gammaLabel != null && gammaProgress < gammaFullText.Length) { gammaProgress += Time.deltaTimeUnscaled * typewriterSpeed; gammaLabel.Text_val = gammaFullText.Substring(0, (int)Utilities.Min(gammaFullText.Length, gammaProgress)); }
                    if (restoreLabel != null && restoreProgress < restoreFullText.Length) { restoreProgress += Time.deltaTimeUnscaled * typewriterSpeed; restoreLabel.Text_val = restoreFullText.Substring(0, (int)Utilities.Min(restoreFullText.Length, restoreProgress)); }
                }
                else
                {
                    if (audioFont != null && audioTypewriterProgress > 0f)
                    {
                        audioTypewriterProgress -= Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Max(0f, audioTypewriterProgress);
                        audioFont.Text_val = audioFullText.Substring(0, length);
                    }

                    if (graphicsFont != null && graphicsTypewriterProgress > 0f)
                    {
                        graphicsTypewriterProgress -= Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Max(0f, graphicsTypewriterProgress);
                        graphicsFont.Text_val = graphicsFullText.Substring(0, length);
                    }

                    if (escFont != null && escTypewriterProgress > 0f)
                    {
                        escTypewriterProgress -= Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Max(0f, escTypewriterProgress);
                        escFont.Text_val = escFullText.Substring(0, length);
                    }

                    if (miniTitleText != null && miniTitleTypewriterProgress > 0f)
                    {
                        miniTitleTypewriterProgress -= Time.deltaTimeUnscaled * typewriterSpeed;
                        int length = (int)Utilities.Max(0f, miniTitleTypewriterProgress);
                        miniTitleText.Text_val = miniTitleFullText.Substring(0, length);
                    }

                    // Slider Labels Closing
                    if (masterLabel != null && masterProgress > 0f) { masterProgress -= Time.deltaTimeUnscaled * typewriterSpeed; masterLabel.Text_val = masterFullText.Substring(0, (int)Utilities.Max(0f, masterProgress)); }
                    if (bgmLabel != null && bgmProgress > 0f) { bgmProgress -= Time.deltaTimeUnscaled * typewriterSpeed; bgmLabel.Text_val = bgmFullText.Substring(0, (int)Utilities.Max(0f, bgmProgress)); }
                    if (sfxLabel != null && sfxProgress > 0f) { sfxProgress -= Time.deltaTimeUnscaled * typewriterSpeed; sfxLabel.Text_val = sfxFullText.Substring(0, (int)Utilities.Max(0f, sfxProgress)); }
                    if (gammaLabel != null && gammaProgress > 0f) { gammaProgress -= Time.deltaTimeUnscaled * typewriterSpeed; gammaLabel.Text_val = gammaFullText.Substring(0, (int)Utilities.Max(0f, gammaProgress)); }
                    if (restoreLabel != null && restoreProgress > 0f) { restoreProgress -= Time.deltaTimeUnscaled * typewriterSpeed; restoreLabel.Text_val = restoreFullText.Substring(0, (int)Utilities.Max(0f, restoreProgress)); }
                }
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
                float currentSysTitleWidth = Utilities.SmoothStep(1400, -572, bgProgress);
                float currentMiniTitleBGWidth = Utilities.SmoothStep(800, -298, bgProgress);
                
                frontBgTrans.Height = (int)currentHeight;
                
                if (systemsPausedTrans != null) systemsPausedTrans.Right = (int)currentSysTitleWidth;
                if (miniTitleBGTrans != null) miniTitleBGTrans.Right = (int)currentMiniTitleBGWidth;
            }

            bool isFullyOpen = true;

            if (isFullyOpen)
            {
                if (coreElements.Count > 2 && coreElements[2] != null) coreElements[2].SetActive(true);
                if (coreElements.Count > 4 && coreElements[4] != null) coreElements[4].SetActive(true);

                if (pages.Count > 0 && pages[0] != null) pages[0].SetActive(isAudioPageSelected);
                if (pages.Count > 1 && pages[1] != null) pages[1].SetActive(!isAudioPageSelected);

                if(isAudioPageSelected)
                {
                    float currentSliderWidth = Utilities.SmoothStep(0, -1161, bgProgress);
                    for (int i = 0; i < 3; i++)
                    {
                        if (sliderTrans != null && i < sliderTrans.Length && sliderTrans[i] != null)
                        {
                            sliderTrans[i].Right = (int)currentSliderWidth;
                        }

                        
                    }
                }
                else
                {
                    float currentGammaSliderWidth = Utilities.SmoothStep(450, -300, bgProgress);
                    if (sliderTrans != null && sliderTrans.Length > 3 && sliderTrans[3] != null)
                    {
                        sliderTrans[3].Right = (int)currentGammaSliderWidth;
                    }
                }

                    float currentEscWidth = Utilities.SmoothStep(540, 450, bgProgress);
                    if (escTrans != null) escTrans.Right = (int)currentEscWidth;


                UpdateTitleText();
            }
            else
            {

                //if (audioSettingsPage != null) audioSettingsPage.SetActive(false);
                //if (graphicsSettingsPage != null) graphicsSettingsPage.SetActive(false);
                if (coreElements.Count > 2 && coreElements[2] != null) coreElements[2].SetActive(false);
            }
            
            if (animationTimer <= 0f && !isOpening)
            {
                isActive = false;
                if (coreElements.Count > 1 && coreElements[1] != null)
                {
                    coreElements[1].SetActive(true);
                }
                this.gameObject.SetActive(false);
            }
        }
    }

    private void UpdateButtonVisuals()
    {
        // Update Audio Button

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
            string targetText = isAudioPageSelected ? "ADJUST AUDIO SETTINGS" : "ADJUST PREFERRED BRIGHTNESS";
            if (miniTitleFullText != targetText)
            {
                miniTitleFullText = targetText;
                miniTitleTypewriterProgress = 0f;
                miniTitleText.Text_val = "";
            }
        }
    }

    public void StartSettingsPopupAnimation(bool opening)
    {
        this.gameObject.SetActive(true);
        isOpening = opening;
        flickerTimer = 0f; // Reset flicker timer

        if (opening)
        {
            audioTypewriterProgress = 0f;
            graphicsTypewriterProgress = 0f;
            escTypewriterProgress = 0f;
            miniTitleTypewriterProgress = 0f;

            if (audioFont != null) audioFont.Text_val = "";
            if (graphicsFont != null) graphicsFont.Text_val = "";
            if (escFont != null) { escFullText = escFont.Text_val; escFont.Text_val = ""; }
            if (miniTitleText != null) miniTitleText.Text_val = "";
            
            // Reset Slider Labels
            masterProgress = 0f; if (masterLabel != null) masterLabel.Text_val = "";
            bgmProgress = 0f; if (bgmLabel != null) bgmLabel.Text_val = "";
            sfxProgress = 0f; if (sfxLabel != null) sfxLabel.Text_val = "";
            gammaProgress = 0f; if (gammaLabel != null) gammaLabel.Text_val = "";
            restoreProgress = 0f; if (restoreLabel != null) restoreLabel.Text_val = "";

            UpdateTitleText();
        }
        else
        {
            // Set progress to current lengths so it can decrease for reverse typewriter
            if (audioFont != null) audioTypewriterProgress = (float)audioFullText.Length;
            if (graphicsFont != null) graphicsFont.Text_val = graphicsFullText; // Ensure full text is there before deleting
            if (graphicsFont != null) graphicsTypewriterProgress = (float)graphicsFullText.Length;
            if (escFont != null) { escFullText = escFont.Text_val; escTypewriterProgress = (float)escFullText.Length; }
            if (miniTitleText != null) miniTitleTypewriterProgress = (float)miniTitleFullText.Length;

            // Initialize Slider Labels for Closing
            if (masterLabel != null) { masterLabel.Text_val = masterFullText; masterProgress = (float)masterFullText.Length; }
            if (bgmLabel != null) { bgmLabel.Text_val = bgmFullText; bgmProgress = (float)bgmFullText.Length; }
            if (sfxLabel != null) { sfxLabel.Text_val = sfxFullText; sfxProgress = (float)sfxFullText.Length; }
            if (gammaLabel != null) { gammaLabel.Text_val = gammaFullText; gammaProgress = (float)gammaFullText.Length; }
            if (restoreLabel != null) { restoreLabel.Text_val = restoreFullText; restoreProgress = (float)restoreFullText.Length; }
        }

        UpdateButtonVisuals();
        isActive = true;
    }

        public void SwitchToPage(bool isAudio)
        {
            isAudioPageSelected = isAudio;

            // Apply immediately if already open
            if (animationTimer >= 1.0f && isOpening)
            {
                if (pages.Count > 0 && pages[0] != null) pages[0].SetActive(isAudio);
                if (pages.Count > 1 && pages[1] != null) pages[1].SetActive(!isAudio);
                UpdateTitleText();
            }

            UpdateButtonVisuals();
        }
    }
}

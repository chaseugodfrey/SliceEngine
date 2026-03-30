using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{


    public class UIAnimation : SliceBehaviour 
    {
        //Animation Settings
        public float duration = 0.5f;
        public int defaultWidth = 1174;
        public int hoverWidth = 773;
        public string textToShow = "";

        public GameObject rectObj;
        public GameObject glitchRectObj;
        public GameObject frontTextObj;
        public GameObject backTextObj;

        private RectTransform rectTransform;
        private SpriteRenderer spriteTrans;
        private FontRenderer frontText;
        private FontRenderer backText;

        private float animationTimer = 0f;
        private bool isHovering = false;

        public bool useGlitch = true;
        private float glitchIntensity = 0f;

        public override void OnCreate()
        {
            if (rectObj != null)
            {
                rectTransform = rectObj.GetComponent<RectTransform>();
                spriteTrans = rectObj.GetComponent<SpriteRenderer>();
            }

            if (frontTextObj != null)
            {
                frontText = frontTextObj.GetComponent<FontRenderer>();
            }

            if (backTextObj != null)
            {
                backText = backTextObj.GetComponent<FontRenderer>();
            }
        }

        public override void OnUpdate(float dt)
        {
            if (isHovering && animationTimer < 1.0f)
            {

                animationTimer += Time.deltaTimeUnscaled / duration;
            }
            else if (!isHovering && animationTimer > 0f)
            {

                animationTimer -= Time.deltaTimeUnscaled / duration;
            }

            animationTimer = Utilities.Clamp(animationTimer, 0f, 1f);

            glitchIntensity = (float)Math.Sin(animationTimer * Math.PI);

            if (rectTransform != null)
            {
                float currentVal = Utilities.SmoothStep(defaultWidth, hoverWidth, animationTimer);
                rectTransform.Right = (int)currentVal;
            }

            if (frontText != null && !string.IsNullOrEmpty(textToShow) && backText != null)
            {
                // Calculate how many characters to show based on the timer
                // We use (int) to truncate the float
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

            if (glitchRectObj != null)
            {
                
                if (isHovering)
                {

                    bool shouldFlicker = SliceRandom.ValueFloat() > 0.98f;
                    glitchRectObj.SetActive(shouldFlicker);

                    
                    int charactersToShow = (int)(animationTimer * textToShow.Length);
                    string displayStr = textToShow.Substring(0, charactersToShow);

                    if (shouldFlicker)
                    {
                        // --- SYNCED CONTENT GLITCH ---
                        // Add multiple random characters during the flicker burst
                        string glitchChars = "X/#_01";
                        for (int i = 0; i < 2; i++)
                        {
                            int charIndex = (int)(SliceRandom.ValueFloat() * glitchChars.Length);
                            displayStr += glitchChars[charIndex];
                        }

                        // --- SYNCED POSITION GLITCH ---
                        RectTransform ghostTrans = glitchRectObj.GetComponent<RectTransform>();
                        RectTransform textGhostTrans = backTextObj.GetComponent<RectTransform>();

                        int direction = (int)(SliceRandom.ValueFloat() * 4);
                        int offset = 15;

                        // Apply directional logic to both Rect and Text Ghost
                        ApplyDirectionalOffset(ghostTrans, rectTransform, direction, offset);
                        ApplyDirectionalOffset(textGhostTrans, null, direction, 10); 

                        if (backText != null) backText.Colour = new Vector4(1.0f, 0.0f, 0.3f, 0.8f);
                    }
                    else
                    {
                        // Standard typewriter glitch (single char during reveal)
                        if (useGlitch && charactersToShow < textToShow.Length)
                        {
                            displayStr += "X/#_01"[(int)(SliceRandom.ValueFloat() * 6)];
                        }

                        // Reset back text position and color
                        ResetGhostPosition(backTextObj.GetComponent<RectTransform>());
                        if (backText != null) backText.Colour = new Vector4(0.8f, 0.8f, 0.8f, 0.5f);
                    }

                    // Apply final strings
                    if (frontText != null) frontText.Text_val = displayStr;
                    if (backText != null) backText.Text_val = displayStr;
                }
            }


            

            
        }

        private void ResetGhostPosition(RectTransform ghostTrans)
        {
            if (ghostTrans == null) return;

            
            ghostTrans.Left = 0;
            ghostTrans.Right = 0;

            ghostTrans.Pos_Y = 0;
        }

        private void ApplyDirectionalOffset(RectTransform trans, RectTransform reference, int dir, int amt)
        {
            if (trans == null) return;
            // If reference is null, we assume we are offsetting from 0 (like backText)
            int baseR = (reference != null) ? reference.Right : 0;
            int baseY = (reference != null) ? reference.Pos_Y : 0;

            if (dir == 0) trans.Left = (reference != null ? reference.Left : 0) - amt;
            else if (dir == 1) trans.Right = baseR - amt;
            else if (dir == 2) trans.Pos_Y = baseY - amt;
            else if (dir == 3) trans.Pos_Y = baseY + amt;
        }

        public void ButtonClickAnim()
        {
            useGlitch = false;
            if (rectObj != null)
            {
                spriteTrans.Colour = (new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
            }

            if (frontText != null && backText != null)
            {
                frontText.Colour = (new Vector4(0.4f, 0.4f, 0.4f, 1.0f));
                backText.Colour = (new Vector4(0.2f, 0.2f, 0.2f, 1.0f));

            }
        }

        public void ResetButton()
        {
            useGlitch = true;
            if (rectObj != null)
            {
                spriteTrans.Colour = (new Vector4(0.3f, 0.3f, 0.3f, 1.0f));
            }

            if (frontText != null && backText != null)
            {
                frontText.Colour = (new Vector4(0.8f, 0.8f, 0.8f, 1.0f));
                backText.Colour = (new Vector4(0.8f, 0.8f, 0.8f, 0.5f));

            }
        }

        public void ButtonHoverState(bool state)
        {
            if(state)
            {
                isHovering = true;
                
            }
            else
            {
                isHovering = false;
            }
        }
    }
}

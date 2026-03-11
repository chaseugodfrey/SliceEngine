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
        public GameObject clickedRectObj;
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

                animationTimer += Time.deltaTime / duration;
            }
            else if (!isHovering && animationTimer > 0f)
            {

                animationTimer -= Time.deltaTime / duration;
            }

            animationTimer = Utilities.Clamp(animationTimer, 0f, 1f);

            glitchIntensity = (float)Math.Sin(animationTimer * Math.PI);

            if (rectTransform != null)
            {
                float currentVal = Utilities.SmoothStep(defaultWidth, hoverWidth, animationTimer);
                rectTransform.Right = (int)currentVal;
            }

            if (clickedRectObj != null)
            {
                // Only allow flickering if we are in the 'hover' state
                if (isHovering)
                {
                    // Probability check: 15% chance to be visible this frame
                    // This creates the high-speed "flicker" effect
                    bool shouldFlicker = SliceRandom.ValueFloat() > 0.95f;
                    clickedRectObj.SetActive(shouldFlicker);

                    if (shouldFlicker)
                    {
                        RectTransform ghostTrans = clickedRectObj.GetComponent<RectTransform>();
                        if (ghostTrans != null)
                        {
                            // Apply a random horizontal offset to create the "split"
                            float splitX = (SliceRandom.ValueFloat() - 0.5f) * 30f;
                            ghostTrans.Right = rectTransform.Right + (int)splitX;
                        }
                    }
                }
                else
                {
                    // Ensure it's hidden when not hovering
                    clickedRectObj.SetActive(false);
                }
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

            if (useGlitch && animationTimer > 0.1f && animationTimer < 0.9f)
            {
                // 1. Create a random displacement for the "ghost" text
                if (SliceRandom.ValueFloat() > 0.85f) // Frequency of the flicker
                {
                    float offsetX = (SliceRandom.ValueFloat() - 0.5f) * 20f;
                    float offsetY = (SliceRandom.ValueFloat() - 0.5f) * 10f;

                    // Shift the back text object if it has a RectTransform
                    RectTransform backTrans = backTextObj.GetComponent<RectTransform>();
                    if (backTrans != null)
                    {
                        // We use small offsets to create the "split" effect
                        backTrans.Right = (int)offsetX;
                        // Assuming your engine has a Top/Y offset
                        // backTrans.Top = (int)offsetY; 
                    }

                    // 2. Flicker the color to a "glitch" hue (e.g., cyan or magenta)
                    if (backText != null)
                        backText.Colour = new Vector4(1.0f, 0.0f, 0.3f, 0.8f); // Cyberpunk Pink
                }
                else
                {
                    // Reset to default shadow state
                    if (backText != null)
                        backText.Colour = new Vector4(0.8f, 0.8f, 0.8f, 0.5f);
                }
            }
        }

        public void ButtonClickAnim()
        {
            if (clickedRectObj != null)
            {
                clickedRectObj.SetActive(true);
            }

            if (frontText != null && backText != null)
            {
                frontText.Colour = (new Vector4(0.4f, 0.4f, 0.4f, 1.0f));
                backText.Colour = (new Vector4(0.2f, 0.2f, 0.2f, 1.0f));

            }
        }

        public void ResetButton()
        {
            if (clickedRectObj != null)
            {
                clickedRectObj.SetActive(false);
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

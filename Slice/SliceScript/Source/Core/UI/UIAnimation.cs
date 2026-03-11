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
        private FontRenderer frontText;
        private FontRenderer backText;

        private float animationTimer = 0f;
        private bool isHovering = false;

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

                // Update the text value with a substring
                frontText.Text_val = textToShow.Substring(0, charactersToShow);
                backText.Text_val = textToShow.Substring(0, charactersToShow);
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

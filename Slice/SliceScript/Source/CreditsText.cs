using System;
using SliceEngine;

namespace SliceEngine
{
    public class CreditsText : SliceBehaviour
    {
        public float scrollSpeed = 500.0f;
        public float fadeSpeed = 1.0f;
        public float displayDuration = 3.0f;

        private GameObject thankYouImage;

        private RectTransform myTransform;

        private SpriteRenderer thankYouSprite;


        private bool creditsOut = false;
        private float exactPosY;
        private float currentAlpha = 0.0f;
        private float endTimer = 0.0f;

        public override void OnCreate()
        {
            myTransform = GetComponent<RectTransform>();

            if(FindGameObjectWithName("ThankYou") != null)
            {
                thankYouImage = FindGameObjectWithName("ThankYou");
                thankYouSprite = thankYouImage.GetComponent<SpriteRenderer>();
            }

            if (myTransform != null)
            {
                // Store the starting Y position
                exactPosY = myTransform.Pos_Y;
            }

            Console.WriteLine("In On Create");
            //SliceLog.Log("Credits script created!");
        }

        public override void OnUpdate(float dt)
        {
            if (myTransform != null)
            {
                if(myTransform.Pos_Y <= 1450.0f)
                {
                    exactPosY += scrollSpeed * dt;

                    myTransform.Pos_Y = (int)exactPosY;

                }
                else
                {
                    creditsOut = true;
                }
            }

            if (creditsOut && thankYouSprite != null)
            {
                if (currentAlpha < 1.0f)
                {
                    // Fading logic
                    currentAlpha += fadeSpeed * dt;
                    if (currentAlpha > 1.0f) currentAlpha = 1.0f;

                    Vector4 color = thankYouSprite.Colour;
                    color.w = currentAlpha;
                    thankYouSprite.Colour = color;
                }
                else
                {
                    
                    endTimer += dt;

                    if (endTimer >= displayDuration)
                    {
                        
                        SceneManager.LoadScene("MenuScene");
                    }
                }
            }

        }
    }
}
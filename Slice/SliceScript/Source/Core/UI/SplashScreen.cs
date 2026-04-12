using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SplashScreen : SliceBehaviour
    {
        private GameObject SplashScreenCanvas;
        private GameObject[] all_images;

        public float growthWidth = 50.0f;
        public float growthHeight = 20.0f;
        public int originalWidth = 300;
        public int originalHeight = 100;

        public float duration = 2.0f;
        private float elapsedTime = 0.0f;
        private float skipDuration = 0.75f;
        private float skipTime = 0.0f;
        private uint curr_img = 1;

        private bool sceneChanged = false;

        public override void OnCreate()
        {
            sceneChanged = false;
            SplashScreenCanvas = FindGameObjectWithName("SplashScreen_Canvas");
            if (SplashScreenCanvas != null)
            {
                all_images = SplashScreenCanvas.GetAllChildren();

                for(int i = 1; i < all_images.Length; i++)
                {
                    SpriteRenderer img_renderer = all_images[i].GetComponent<SpriteRenderer>();

                    img_renderer.SetEnabled(true);

                    Vector4 color = img_renderer.Colour;
                    color.w = 0.0f;
                    img_renderer.Colour = color;
                }
            }
         }

        public override void OnUpdate(float dt)
        {
            if(all_images.Length > 0 && curr_img < all_images.Length)
            {
                if(elapsedTime < duration)
                {
                    elapsedTime += dt;
                    float normalizedTime = elapsedTime / duration;

                    if (normalizedTime > 1.0f)
                        normalizedTime = 1.0f;

                    //Alpha: fade in then fade out
                    float mid = 1f - Math.Abs(2f * normalizedTime - 1f);
                    float alpha = Utilities.EaseInOut(mid);

                    //Size
                    float sizeT = Utilities.EaseIn(normalizedTime);

                    ChangeAlpha(all_images[curr_img], alpha, sizeT);
                }
                else
                {
                    elapsedTime = 0.0f;
                    curr_img++;
                }
            }
            else
            {
                StartGameMenu();
                //SliceLog.Log("all img fin, start game");
            }

            if(Input.IsKeyPressed(Keys.KEY_SPACEBAR))
            {
                elapsedTime = 0.0f;
                ChangeAlpha(all_images[curr_img], 0.0f, 0.0f);
                curr_img++;
                if (curr_img >= all_images.Length)
                {
                    StartGameMenu();
                }
            }
        }

        public void StartGameMenu()
        {
            SceneManager.LoadScene("MenuScene");
        }

        public void ChangeAlpha(GameObject obj, float alpha, float sizeT)
        {
            SpriteRenderer img_renderer = obj.GetComponent<SpriteRenderer>();

            Vector4 color = img_renderer.Colour;
            color.w = alpha;
            img_renderer.Colour = color;

            RectTransform rect = obj.GetComponent<RectTransform>();


            SliceLog.Log("Base Size: " + sizeT.ToString());

            if (rect != null)
            { 
                rect.Width = (int)originalWidth + (int)(sizeT * growthWidth);
                rect.Height = (int)originalHeight + (int)(sizeT * growthHeight);
            }
        }

        public void ChangeDimension(GameObject obj, float width, float height)
        {

            //RectTransform rect = obj.GetComponent<RectTransform>();
            //if (rect != null)
            //{
            //    rect.Width += (int)width;
            //    rect.Height += (int)height;
            //}
        }
    }
}

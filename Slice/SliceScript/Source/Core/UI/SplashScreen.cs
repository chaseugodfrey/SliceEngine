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

        private float duration = 2.0f;
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

                    float mid = 1f - Math.Abs(2f * normalizedTime - 1f);

                    float alpha = Utilities.EaseInOut(mid);



                    ChangeAlpha(all_images[curr_img], alpha);
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

            if(Input.IsKeyDown(Keys.KEY_SPACEBAR))
            { 
                if (skipTime < skipDuration)
                {
                    skipTime += dt;
                }
                else
                {
                    if (!sceneChanged)
                        StartGameMenu();

                    sceneChanged = true;
                    //SliceLog.Log("skip fin, start game");
                }
            }
            else
            {
                skipTime = 0.0f;
            }
        }

        public void StartGameMenu()
        {
            SceneManager.LoadScene("MenuScene");
        }

        public void ChangeAlpha(GameObject obj, float alpha)
        {
            SpriteRenderer img_renderer = obj.GetComponent<SpriteRenderer>();

            Vector4 color = img_renderer.Colour;
            color.w = alpha;
            img_renderer.Colour = color;

            RectTransform rect = obj.GetComponent<RectTransform>();
            if (rect != null)
            {
                rect.Width += 3;
                rect.Height += 1;
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

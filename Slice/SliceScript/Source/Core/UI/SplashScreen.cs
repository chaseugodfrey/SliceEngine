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
        private uint curr_img = 0;

        private bool sceneChanged = false;

        public override void OnCreate()
        {
            sceneChanged = false;
            SplashScreenCanvas = FindGameObjectWithName("SplashScreen_Canvas");
            if (SplashScreenCanvas != null)
            {
                all_images = SplashScreenCanvas.GetAllChildren();

                foreach(GameObject img in all_images)
                {
                    SpriteRenderer img_renderer =img.GetComponent<SpriteRenderer>();

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

                    SpriteRenderer img_renderer = all_images[curr_img].GetComponent<SpriteRenderer>();

                    Vector4 color = img_renderer.Colour;
                    color.w = alpha;
                    img_renderer.Colour = color;
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
    }
}

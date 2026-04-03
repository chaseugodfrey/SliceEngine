using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SettingsCloseBGAnimation : SliceBehaviour
    {
        private bool playAnim = false;

        private SpriteAnimator bgAnimation;

        public GameObject MainMenuCanvas;

        public SettingsBorderAnimation borderAnim;

        public override void OnCreate()
        {
            bgAnimation = GetComponent<SpriteAnimator>();

        }

        public override void OnUpdate(float dt)
        {
            if (bgAnimation != null && playAnim)
            {
                bgAnimation.IsPlaying = true;
                //AudioSettings.PlaySFX("PauseTransitionOut");
            }



        }

        public override void OnSpriteAnimLoop()
        {

        }

        public override void OnSpriteAnimStop()
        {
            if (bgAnimation != null && playAnim)
            {
                bgAnimation.IsPlaying = false;
                playAnim = false;

                // The sequence is finished! Show the menu again.
                if (MainMenuCanvas != null)
                {
                    MainMenuCanvas.SetActive(true);
                }

                this.gameObject.SetActive(false);
            }
        }

        public void StartSettingsBGAnimation(bool isOpening)
        {
            playAnim = isOpening;
            this.gameObject.SetActive(true);
        }
    }
}

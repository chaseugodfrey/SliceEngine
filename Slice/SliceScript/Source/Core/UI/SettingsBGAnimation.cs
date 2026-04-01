using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SettingsBGAnimation : SliceBehaviour
    {
        private bool playAnim = true;
        private SpriteAnimator bgAnimation;
        public SettingsBorderAnimation borderAnim;
        private GameObject settingsPopup; // The object holding the Animation script
        

        public override void OnCreate()
        {
            bgAnimation = GetComponent<SpriteAnimator>();
        }

        public override void OnUpdate(float dt)
        {
            //if (bgAnimation != null)
            //{
            //    if (playAnim && !bgAnimation.IsPlaying)
            //    {
                    
            //        bgAnimation.IsPlaying = true;
            //    }

            //}

        }

        public override void OnSpriteAnimLoop()
        {
            
        }

        public override void OnSpriteAnimStop()
        {
            if (bgAnimation != null)
            {
                if(playAnim)
                {
                    
                    bgAnimation.IsPlaying = false;
                    playAnim = false;
                }
                
                    
                if (borderAnim != null)
                {
                    borderAnim.StartSettingsPopupAnimation(true);
                }
                

            }
        }

        //public void StartSettingsBGAnimation(bool isOpening)
        //{
        //    playAnim = isOpening;
        //    this.gameObject.SetActive(true);
        //}
    }
}

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
        private bool playAnim = false;
        
        private SpriteAnimator bgAnimation;
        
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
                
            }

           
            
        }

        public override void OnSpriteAnimLoop()
        {
            
        }

        public override void OnSpriteAnimStop()
        {
            if (bgAnimation != null)
            {
                if (playAnim)
                {
                    bgAnimation.IsPlaying = false;
                    playAnim = false;
                    
                    if (borderAnim != null)
                    {
                        borderAnim.StartSettingsPopupAnimation(true);
                    }
                    this.gameObject.SetActive(false);
                }
            }
        }

        public void StartSettingsBGAnimation(bool isOpening)
        {
            playAnim = isOpening;
            
        }
    }
}

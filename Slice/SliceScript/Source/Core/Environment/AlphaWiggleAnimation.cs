using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{

    public class AlphaWiggleAnimation : SliceBehaviour
    {
        public bool active = true;
        public float rate = 1f;
        public GameObject rendererObject;
        private Renderer storedRenderer;
        public float MaxWiggle = 1f;
        public float MinWiggle = 1f;
        public bool random = false;
        private bool lowerAlpha = false;
        //public float floatingAmp = 10f;
        //public float floatingFreq = 10f;
        private float count = 0f;

        public override void OnCreate()
        {
            base.OnCreate();

        }

        public void Reset()
        {
            active = false;

            if (rendererObject == null)
            {
                //rendererObject = gameObject;
                return;
            }

            if (storedRenderer == null && rendererObject.HasComponent<Renderer>())
            {
                storedRenderer = rendererObject.GetComponent<Renderer>();
            }
            if (storedRenderer == null)
            {
                return;
            }
            if(!rendererObject.HasComponent<Renderer>())
            {
                return;
            }

            Vector4 color = storedRenderer.GetColor();

            color.w = MaxWiggle;

            //storedRenderer = gameObject.GetComponent<Renderer>();

            storedRenderer.SetColor(color);

        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (storedRenderer == null)
            {
                storedRenderer = rendererObject.GetComponent<Renderer>();
            }

            if (!active)
            {
                if (storedRenderer != null)
                {
                    Vector4 color = storedRenderer.GetColor();
                    color.w = 0.0f;
                    storedRenderer.SetColor(color);
                }
                return;
            }



            count += dt;

            //float calc = (float)(floatingAmp * Math.Sin( floatingFreq * cycle));

            //transform.Scale =  storedTransform + new Vector3( 0, 0, 0 );

            if (count >= 1/rate)
            {
                count -= 1 / rate;

                Vector4 color = storedRenderer.GetColor();

                if (random)
                {
                    color.w = SliceRandom.RangeFloat(MinWiggle, MaxWiggle);
                }
                else if (lowerAlpha)
                {
                    color.w = MaxWiggle;
                    lowerAlpha = false;
                }
                else if (!lowerAlpha)
                {
                    color.w = MinWiggle;
                    lowerAlpha = true;
                }

                    storedRenderer.SetColor(color);
            }
        }
    }
}

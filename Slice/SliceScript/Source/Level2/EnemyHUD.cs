using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyHUD : SliceBehaviour
    {
        public GameObject healthBar;

        public override void OnCreate()
        {
            HideBar(true);
        }

        public override void OnUpdate(float dt)
        {
            
        }

        public void SetHealth(float input)
        {
            healthBar.GetComponent<Slider>().SetValue(input);
        }

        public void HideBar(bool toHide)
        {
            GameObject[] children = gameObject.GetAllChildren();

            float alpha = toHide ? 0.0f : 1.0f;

            foreach (GameObject go in children)
            {
                if (go.HasComponent<SpriteRenderer>())
                {
                    Vector4 Color = go.GetComponent<SpriteRenderer>().Colour;
                    Color.w = alpha;
                    go.GetComponent<SpriteRenderer>().Colour = Color;
                }
                else if (go.HasComponent<FontRenderer>())
                {
                    Vector4 Color = go.GetComponent<FontRenderer>().Colour;
                    Color.w = alpha;
                    go.GetComponent<FontRenderer>().Colour = Color;
                }
            }
        }
    }
}

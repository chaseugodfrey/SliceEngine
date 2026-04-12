using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Lvl3EnemyHUD : SliceBehaviour
    {
        public GameObject healthBar;
        public GameObject shieldBar;

        public override void OnCreate()
        {
            
        }

        public override void OnUpdate(float dt)
        {
            
        }

        public void SetHealth(float input)
        {
            healthBar.GetComponent<Slider>().SetValue(input);
        }

        public void SetShield(float input)
        {
            shieldBar.GetComponent<Slider>().SetValue(input);
        }
    }
}

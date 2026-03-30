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
            
        }

        public override void OnUpdate(float dt)
        {
            
        }

        public void SetHealth(float input)
        {
            healthBar.GetComponent<Slider>().SetValue(input);
        }
    }
}

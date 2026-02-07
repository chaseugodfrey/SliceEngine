using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SliceEngine;

namespace SliceEngine
{
    public class SceneTransition : SliceBehaviour
    {
  
        

        public override void OnCreate()
        {
            SpriteRenderer sr = GetComponent<SpriteRenderer>();

            if (sr != null)
            {
                SceneManager.RegisterTransition(this, sr);
            }
        }

        public override void OnUpdate(float dt)
        {
            

        }

        
    }
}

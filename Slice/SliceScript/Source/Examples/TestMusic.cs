using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestMusic : SliceBehaviour
    {

        public override void OnCreate()
        {
            

            AudioSource myAudio = gameObject.GetComponent<AudioSource>();
            
            if(myAudio != null)
            {

                myAudio.Play();
            }

        }
    }
}

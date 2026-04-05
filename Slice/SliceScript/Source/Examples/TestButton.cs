using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestMovingPlatform : SliceBehaviour
    {
        GameObject player;

        public override void OnAwake()
        {
            player = gameObject.FindGameObjectWithName("PlayerNewFinal");
        }
        public override void OnCollideStay(uint other)
        {
            if (other == player.mID)
            {
                
            }
        }

    }
}

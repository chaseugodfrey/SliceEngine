using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class PlayerHitBox : SliceBehaviour
    {
        GameObject swordHitBox;

        public void EnableBox(string str)
        {
            SliceLog.Log("Testing :" + str);
        }

        public void DisableBox(string str) 
        {
            SliceLog.Log("Testing :" + str);
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestButton : SliceBehaviour
    {
        int test = 0;

        public override void OnButtonClick()
        {
            test++;

            SliceLog.Log("Test "+ test);
        }

        
    }
}

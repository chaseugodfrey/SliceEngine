using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestButtonSlider : SliceBehaviour
    {
        public override void OnButtonHover()
        {
            SliceLog.Log("On Hover");
        }


    }
}

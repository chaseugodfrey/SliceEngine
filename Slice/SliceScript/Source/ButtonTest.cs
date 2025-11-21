using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class Button : SliceBehaviour
    {
        public override void OnButtonClick()
        {
            SliceLog.Log("Button CLick");
        }

        public override void OnButtonRelease()
        {
            SliceLog.Log("Button Release");
        }
    }
}

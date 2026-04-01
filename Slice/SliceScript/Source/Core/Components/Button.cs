using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Button : Component
    {
        public Button() { }
        public Button(GameObject entity)
        {
            gameObject = entity;
        }

        public void SetEnabled(bool enabled)
        {
            FunctionCalls.Button_SetEnabled(gameObject.mID, enabled);
        }
    }
}

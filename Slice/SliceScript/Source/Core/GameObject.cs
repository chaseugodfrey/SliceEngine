using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    // if GO is component, it cannot getcomponent, if GO is SliceBehaviour, it will show up under scripts when it shouldnt :(
    public class GameObject : Component
    {
        public string name
        {
            get => name;
            set
            {
                name = value;
            }
        }

        public string tag
        {
            get => tag;
            set
            {
                tag = value;
            }
        }

    }
}

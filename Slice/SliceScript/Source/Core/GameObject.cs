using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class GameObject : SliceBehaviour
    {
        public string tag
        { 
            get
            {
                return tag;
            }
            set
            {
                tag = value;
            }
        }

    }
}

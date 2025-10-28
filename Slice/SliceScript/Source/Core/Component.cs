using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public abstract class Component
    {
        public SliceBehaviour Entity { get; internal set; }
    }
}


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public interface IInitializable
    {
        /// <summary>
        /// Use this as a replacement for Awake()
        /// </summary>
        void Initialize();
    }
}

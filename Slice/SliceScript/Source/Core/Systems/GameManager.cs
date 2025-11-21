using SliceEngine;
using SliceScript.Source.Core.Systems;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceScript.Source.Core
{
    public class GameManager : SliceBehaviour
    {
        private List<IInstanceable> instanceables = new List<IInstanceable>();

        public override void OnAwake()
        {
            base.OnAwake();
            foreach (IInstanceable instanceables in instanceables) instanceables.OnInstanced();
        }
        public void InitializeReference(IInstanceable instanceables)
        {
            instanceables.Add(instanceables);
        }
    }
}

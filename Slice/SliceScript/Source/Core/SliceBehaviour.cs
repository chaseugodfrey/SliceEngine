using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SliceBehaviour : Component
    {
        public uint mID;
        public Transform transform;

        protected SliceBehaviour()
        {
            mID = 0;
        }
        
        ~SliceBehaviour()
        {
            mID = 0;
        }

        internal SliceBehaviour(uint id)
        {
            mID = id;
        }

        public virtual void OnAwake() { }
        //public virtual void OnEnable() { }
        public virtual void OnCreate() { }
        public virtual void OnUpdate(float dt) { }
        //public virtual void OnDestroy() { }
        public virtual void OnFixedUpdate(float dt) { }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
        }
        public T GetComponent<T>() where T : Component, new()
        {
            T component = new T() { Entity = this };
            return component;
        }
    }
}

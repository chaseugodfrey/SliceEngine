using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SliceBehaviour : Component
    {
        // public uint mID;
        // public Transform transform;

        protected SliceBehaviour()
        {
            gameObject = new GameObject(0);
            //mID = 0;
        }
        
        ~SliceBehaviour()
        {
            gameObject.mID = 0;
           // mID = 0;
        }

        internal SliceBehaviour(uint id)
        {
            gameObject.mID = id;
           // mID = id;
          //  Entity.mID = mID;
        }

        public virtual void OnAwake() { }
        //public virtual void OnEnable() { }
        public virtual void OnCreate() { }
        public virtual void OnUpdate(float dt) { }
        //public virtual void OnDestroy() { }
        public virtual void OnFixedUpdate(float dt) { }

        public virtual void OnCollideEnter(uint other) {  }

        public virtual void OnCollideStay(uint other) { }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return FunctionCalls.Entity_HasComponent(gameObject.mID, componentType);
        }
        public new T GetComponent<T>() where T : Component
        {
            var ctor = typeof(T).GetConstructor(new[] { typeof(GameObject) });
            if (ctor == null)
                throw new InvalidOperationException(
                    $"Type {typeof(T).Name} must declare a public constructor {typeof(T).Name}({nameof(GameObject)})");

            T component = (T)ctor.Invoke(new object[] { this.gameObject });
            component.gameObject = gameObject;
            return component;
        }

        public T As<T>() where T : Component, new()
        {
            string baseClassName = typeof(T).Name;
            //CMConsole.Log($"Trying to retrieve script instance of {baseClassName} for entity {mID}");
            Object scriptInstance = FunctionCalls.GetScriptInstance(gameObject.mID, baseClassName);

            if (scriptInstance == null)
            {
                //  CMConsole.Log($"Script instance is null");
                return null;
            }

            return scriptInstance as T;
        }


        public bool Has<T>() where T : Component, new()
        {
            string baseClassName = typeof(T).Name;

            return FunctionCalls.HasScriptInstance(gameObject.mID, baseClassName);
        }


        public GameObject CreateGameObject(string prefabName)
        {
            GameObject entity = new GameObject(FunctionCalls.CreateNewGameObject(prefabName));

            return entity;
        }

        public void Destroy()
        {
            FunctionCalls.Destroy(gameObject.mID);
        }

                public void StartCoroutine(IEnumerator routine)
        {
            if (routine != null)
                CoroutineManager.StartCoroutine(routine, this);
        }

        public void StopAllCoroutines()
        {
            CoroutineManager.StopAllCoroutines(this);
        }
    }
}

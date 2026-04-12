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

        public virtual void OnEntityDestroy(uint id) { }
        public virtual void OnUpdate(float dt) { }
        //public virtual void OnDestroy() { }
        public virtual void OnFixedUpdate(float dt) { }

        public virtual void OnLateUpdate(float dt) { }
        public virtual void OnCollideEnter(uint other) {  }

        public virtual void OnCollideStay(uint other) { }

        public virtual void OnCollideExit(uint other) { }

        public virtual void OnTriggerEnter(uint other) { }

        public virtual void OnTriggerStay(uint other) { }

        public virtual void OnTriggerExit(uint other) { }

        public virtual void OnEnabled() { }

        public virtual void OnDisabled() { }

        public virtual void OnButtonClick() { }
        public virtual void OnButtonHover() { }
        public virtual void OnButtonExitHover() { }
        public virtual void OnButtonRelease() { }
        public virtual void OnSliderValue(float value) { }
        public virtual void OnSpriteAnimStop() { }
        public virtual void OnSpriteAnimLoop() { }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return FunctionCalls.Entity_HasComponent(gameObject.mID, componentType);
        }
        public new T GetComponent<T>() where T : Component
        {
            Type componentType = typeof(T);
            if (!FunctionCalls.Entity_HasComponent(gameObject.mID, componentType))
            {
                return null;
            }


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

        public GameObject CloneGO(string goName)
        {
            GameObject entity = new GameObject(FunctionCalls.CloneGO(goName));

            return entity;
        }

        public GameObject[] FindGameObjectsWithTag(string tag)
        {
            return gameObject.FindGameObjectsWithTag(tag);
        }

        public GameObject FindGameObjectWithName(string name)
        {
            return gameObject.FindGameObjectWithName(name);
        }

        public GameObject FindGameObjectWithID(uint id)
        {
            return gameObject.FindGameObjectWithID(id);
        }
        public GameObject FindGameObjectWithTag(string tag)
        {
            uint entityIDs = FunctionCalls.Entity_FindEntityWithTag(tag);
            GameObject gameObject = new GameObject(entityIDs);

            return gameObject;
        }
        public void Destroy()
        {
            StopAllCoroutines();
            FunctionCalls.Destroy(gameObject.mID);
        }

        public Coroutine StartCoroutine(IEnumerator routine)
        {
            if (routine != null)
            {
                return CoroutineManager.StartCoroutine(routine, this);
            }
            return null;
        }

        public void StopAllCoroutines()
        {
            CoroutineManager.StopAllCoroutines(this);
        }

        public void SetActive(bool active)
        {
            FunctionCalls.Entity_SetActive(gameObject.mID, active);
        }

        public bool IsActive()
        {
                return FunctionCalls.Entity_IsActive(gameObject.mID);
        }
    }
}

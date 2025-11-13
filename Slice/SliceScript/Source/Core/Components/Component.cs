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
        //public SliceBehaviour Entity { get; internal set; }
        public GameObject gameObject { get; internal set; }

        //  // Unique name or tag (like Unity's GameObject.tag)
        //  public string Tag
        //  {
        //      //get => Entity?.Tag ?? string.Empty;
        //      //set
        //      //{
        //      //    if (Entity != null)
        //      //        Entity.Tag = value;
        //      //}
        //  }

        // Whether the entity is active in the scene
        public bool activeSelf { get; set; } = true;

        // Whether this component is enabled (can override in derived types)
        public bool enabled { get; set; } = true;

        // Transform shortcut (like Unity's transform)
        public Transform transform => gameObject?.GetComponent<Transform>();

        // Get another component on the same entity
        public T GetComponent<T>() where T : Component
        {
            return gameObject?.GetComponent<T>();
        }

        // Get component in children
        //public T GetComponentInChildren<T>() where T : Component
        //{
        //    return gameObject.GetComponentInChildren<T>();
        //}

        // Send a message to the entity
        //public void SendMessage(string methodName, object parameter = null)
        //{
        //    gameObject.SendMessage(methodName, parameter);
        //}
    }
}


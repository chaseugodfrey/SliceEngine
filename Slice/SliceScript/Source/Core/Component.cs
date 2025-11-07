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

        // Unique name or tag (like Unity's GameObject.tag)
        public string Tag
        {
            get => Entity?.Tag ?? string.Empty;
            set
            {
                if (Entity != null)
                    Entity.Tag = value;
            }
        }

        // Alias for Unity-like style
        public SliceBehaviour gameObject => Entity;

        // Whether the entity is active in the scene
        public bool ActiveSelf => Entity?.ActiveSelf ?? false;

        // Whether this component is enabled (can override in derived types)
        public bool Enabled { get; set; } = true;

        // Transform shortcut (like Unity's transform)
        public Transform Transform => Entity?.GetComponent<Transform>();

        // Get another component on the same entity
        public T GetComponent<T>() where T : Component
        {
            return Entity?.GetComponent<T>();
        }

        // Get component in children
        public T GetComponentInChildren<T>() where T : Component
        {
            return Entity?.GetComponentInChildren<T>();
        }

        // Send a message to the entity (optional Unity-like behavior)
        public void SendMessage(string methodName, object parameter = null)
        {
            Entity?.SendMessage(methodName, parameter);
        }        
    }
}


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class PopupTriggerBox : SliceBehaviour
    {
        public string dialogueBoxObject;
        public bool triggered = false;
        public int popupIndex = 0;

        public override void OnUpdate(float dt)
        {
            
        }

        public override void OnTriggerStay(uint other)
        {
            GameObject hit = gameObject.FindGameObjectWithID(other);
            if (hit.Has<PlayerController>() && Bootstrap.Player == hit.As<PlayerController>() && !triggered)
            {
                if (Input.IsKeyPressed(Keys.KEY_F))
                {
                    triggered = true;
                    if (dialogueBoxObject != string.Empty)
                    {
                        GameObject dialogueBox = CreateGameObject(dialogueBoxObject);
                        if (dialogueBox != null)
                        {
                            // set up dialogue box here
                        }
                    }
                }
            }
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class PlayerInteractTriggerBox : SliceBehaviour
    {
        public GameObject cinematicPlayer;
        public bool _enabled = false;

        public override void OnUpdate(float dt)
        {
            if (_enabled)
            {
                // if interact
                if (Input.IsKeyPressed(Keys.KEY_F))
                {
                    cinematicPlayer.As<PlayerCinematic>().StartCinematicAnimation();
                }
            }
        }

        public override void OnTriggerEnter(uint other)
        {
            GameObject hit = gameObject.FindGameObjectWithID(other);

            if (hit.Has<PlayerController>() && Bootstrap.Player == hit.As<PlayerController>() && !_enabled)
            {
                _enabled = true;
            }
        }

        public override void OnTriggerExit(uint other)
        {
            GameObject hit = gameObject.FindGameObjectWithID(other);

            if (hit.Has<PlayerController>() && Bootstrap.Player == hit.As<PlayerController>() && _enabled)
            {
                _enabled = false;
            }

        }

    }
}

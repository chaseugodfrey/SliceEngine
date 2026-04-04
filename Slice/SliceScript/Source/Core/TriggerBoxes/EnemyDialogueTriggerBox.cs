using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemyDialogueTriggerBox : SliceBehaviour
    {
        // reference to the enemy to toggle active
        public GameObject enemy;

        public int setOfThisTrigger = 0;

        private bool _done = false;
        private bool _enabled = false;
        private bool enterPressed = false;

        public override void OnCreate()
        {
            // set it to inactive first
            enemy.As<Projectile_Spawner>().active = false;
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (_enabled && !_done)
            {
                if (Input.IsKeyDown(Keys.KEY_F) && enterPressed == false)
                {
                    enterPressed = true;
                    if (!Bootstrap.HUDManager.PlayDialogueForLevel(setOfThisTrigger, Bootstrap.HUDManager.currentScene, true, true))
                    {
                        _done = true;
                    }

                }

                if (Input.IsKeyReleased(Keys.KEY_F) && enterPressed == true)
                {
                    enterPressed = false;
                }
            }
        }

        public override void OnTriggerEnter(uint other)
        {
            base.OnTriggerEnter(other);
            SliceLog.Log("Dialogue Trigger Enter called");
            //base.OnTriggerEnter(other);

            GameObject hit = gameObject.FindGameObjectWithID(other);

            if (hit.Has<PlayerController>() && Bootstrap.Player == hit.As<PlayerController>() && !_enabled)
            {
                //console.writeline("Enabled");
                _enabled = true;
                Bootstrap.HUDManager.PlayDialogueForLevel(setOfThisTrigger, Bootstrap.HUDManager.currentScene, true, true);
            }
        }
    }
}

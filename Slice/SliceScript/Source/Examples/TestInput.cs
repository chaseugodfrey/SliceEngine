using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestInput : SliceBehaviour
    {
        public int buttonType = 0;

        public override void OnUpdate(float dt)
        {
            //if (Input.IsKeyPressed(Keys.KEY_P))
            //{
            //    Cursor.state = Cursor.STATE.DISABLED;
            //}

            //if (Input.IsKeyPressed(Keys.KEY_O))
            //{
            //    Cursor.state = Cursor.STATE.DEFAULT;
            //}

            //if (Input.IsKeyDown(Keys.KEY_0))
            //{
            //    Bootstrap.HUDManager.GameWinScreen();
            //}

            //if (Input.IsKeyDown(Keys.KEY_9))
            //{
            //    Bootstrap.HUDManager.GameLoseScreen();
            //}

            //var mouseDelta = Input.GetMouseDelta();

            //transform.Position += new Vector3(mouseDelta.x, mouseDelta.y, 0) * Time.deltaTime;
            //transform.Position = new Vector3(
            //    Utilities.Clamp(transform.Position.x, -20.0f, 20.0f),
            //    Utilities.Clamp(transform.Position.y, -20.0f, 20.0f),
            //    0);

            
        }

        public override void OnButtonClick()
        {
            if(buttonType == 1)
            {
                Bootstrap.LevelDirector.LoadNextLevel();
            }
            else
            {
                Bootstrap.LevelDirector.RestartLevel();
            }
        }
    }
}

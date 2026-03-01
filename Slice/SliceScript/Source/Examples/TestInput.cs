using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestInput : SliceBehaviour
    {
        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_P))
            {
                Cursor.state = Cursor.STATE.DISABLED;
            }

            if (Input.IsKeyPressed(Keys.KEY_O))
            {
                Cursor.state = Cursor.STATE.DEFAULT;
            }

            var mouseDelta = Input.GetMouseDelta();

            transform.Position += new Vector3(mouseDelta.x, mouseDelta.y, 0) * Time.deltaTime;
            transform.Position = new Vector3(
                Utilities.Clamp(transform.Position.x, -20.0f, 20.0f),
                Utilities.Clamp(transform.Position.y, -20.0f, 20.0f),
                0);
        }
    }
}

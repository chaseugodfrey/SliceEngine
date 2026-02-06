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
            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_LEFT))
            {
                //SliceLog.Log("LMB CLICKED");
            }
            if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_LEFT))
            {
                //SliceLog.Log("LMB PRESSED");
            }
            if (Input.IsMouseReleased(MouseButtons.MOUSE_BUTTON_LEFT))
            {
                //SliceLog.Log("LMB RELEASED");
            }

            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_RIGHT))
            {
                //SliceLog.Log("RMB CLICKED");
            }
            if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_RIGHT))
            {
                //SliceLog.Log("RMB PRESSED");
            }
            if (Input.IsMouseReleased(MouseButtons.MOUSE_BUTTON_RIGHT))
            {
                //SliceLog.Log("RMB RELEASED");
            }

            if (Input.IsMouseDown(MouseButtons.MOUSE_BUTTON_WHEEL))
            {
                //SliceLog.Log("WHEEL CLICKED");
            }
            if (Input.IsMousePressed(MouseButtons.MOUSE_BUTTON_WHEEL))
            {
                //SliceLog.Log("WHEEL PRESSED");
            }
            if (Input.IsMouseReleased(MouseButtons.MOUSE_BUTTON_WHEEL))
            {
                //SliceLog.Log("WHEEL RELEASED");
            }
        }
    }
}

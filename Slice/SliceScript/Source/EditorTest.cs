using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EditorTest : SliceBehaviour
    {
        //public bool boolTest = false;
        public GameObject GOtest;
        public float floatTest = 0.0f;
        public string stringTest;
        public int intTest = 0;
        public bool boolTest = true;
        //public Prefab prefabObj = new Prefab();

        //public List<int> intList = new List<int>();
        //public List<GameObject> goListTest = new List<GameObject>();
        //public Vector3 vec3test;
        public override void OnUpdate(float dt)
        {
            if(Input.IsKeyPressed(Keys.KEY_O))
            {
                //Input.SetCursorState(Cursor.STATE.DEFAULT);

                SliceLog.Console(floatTest + "from " + GOtest.mID);
            }
        }

        public override void OnButtonClick()
        {
            SliceLog.Console("BUTTS");
            Input.SetCursorState(Cursor.STATE.DISABLED);
        }

        public override void OnEnabled()
        {
            SliceLog.Console("On Enabled");
        }

        public override void OnDisabled()
        {
            SliceLog.Console("On Disable");
        }

        //public override void OnCollideEnter(uint other)
        //{
        //    SliceLog.Console("Collide enter with", other);
        //}

        //public override void OnCollideStay(uint other)
        //{
        //    SliceLog.Console("Collide stay with", other);
        //}
    }
}

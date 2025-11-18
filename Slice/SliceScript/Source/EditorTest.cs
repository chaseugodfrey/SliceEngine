using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EditorTest : SliceBehaviour
    {
        public bool boolTest = false;
        public int intTest = 2;
        public float floatTest =  5.0f;
        public string testString = "Test";
        public int[] intArray = {1,3,5,7};
        public string[] stringArray = { "Test1", "Test2", "Test3" };
        public override void OnUpdate(float dt)
        {
            SliceLog.Log("Bool: " + boolTest);
            SliceLog.Log("Int: " + intTest);
            SliceLog.Log("Float: " + floatTest);
            SliceLog.Log(testString);

            int i = 0;
            foreach (float var in intArray)
            {
                SliceLog.Log("Element " + i  +": "+ var);
            }

            foreach (string var in stringArray)
            {
                SliceLog.Log("Element " + i + ": " + var);
            }
        }
    }
}

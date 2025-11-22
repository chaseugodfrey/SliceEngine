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
        public int[] intArray = {1,3};
        public float[] floatArray = {1.0f,2.0f};
        public string[] stringArray = { "Test1", "Test2", "Test3" };
        public List<float> floatList = new List<float>{ 0.5f,2.0f };
        public List<int> intList = new List<int>{ 7,8 };
        public List<string> stringList = new List<string>{"Test","Test2"};
        public override void OnUpdate(float dt)
        {

            int i = 0;
            foreach(float var in floatList)
            {
                SliceLog.Log("Float List Element " + i + ": " + var);
                i++;
            }

            int j = 0;
            foreach (string var in stringList)
            {
                SliceLog.Log("String List Element " + i + ": " + var);
                j++;
            }
            int k = 0;
            foreach (int var in intList)
            {
                SliceLog.Log("Int List Element " + i + ": " + var);
                k++;
            }
        }
    }
}

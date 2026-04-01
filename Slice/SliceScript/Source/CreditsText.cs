using System;
using SliceEngine;

namespace SliceEngine
{
    public class CreditsText : SliceBehaviour
    {
        public float scrollSpeed = 500.0f;

        private RectTransform myTransform;

        private float exactPosY;

        public override void OnCreate()
        {
            myTransform = GetComponent<RectTransform>();

            if (myTransform != null)
            {
                // Store the starting Y position
                exactPosY = myTransform.Pos_Y;
            }

            Console.WriteLine("In On Create");
            //SliceLog.Log("Credits script created!");
        }

        public override void OnUpdate(float dt)
        {
            if (myTransform != null)
            {
                exactPosY += scrollSpeed * dt;

                myTransform.Pos_Y = (int)exactPosY;
            }
        }
    }
}
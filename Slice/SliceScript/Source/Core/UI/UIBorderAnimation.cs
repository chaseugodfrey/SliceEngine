using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class UIBorderAnimation : SliceBehaviour
    {
        public bool startPos = false;
        public bool endPos = false;
        public Vector3 startPosValue = new Vector3(0f, 0f, 0f);
        public Vector3 endPosValue = new Vector3(0f, 0f, 0f);
        public GameObject topOrBottomLine;
        public GameObject sideLine;

        // Property names based on your requirements
        public string horizontalPropName = ""; // e.g., "Left" or "Right"
        public int topLineHiddenValue;
        public int sideLineHiddenValue;
        public int topLineShownValue;
        public int sideLineShownValue;

        private RectTransform trans;
        private RectTransform hLineTrans;
        private RectTransform sLineTrans;

        private float animTimer = 0f;
        public float animSpeed = 0.5f;
        public bool triggerAnimation = false;
        public override void OnCreate()
        {
            if(topOrBottomLine != null)
            {
                hLineTrans = topOrBottomLine.GetComponent<RectTransform>();
            }

            if (sideLine != null)
            {
                sLineTrans = sideLine.GetComponent<RectTransform>();
            }
        }

        public override void OnUpdate(float dt)
        {
            //SliceLog.Log("Start");
            if (!triggerAnimation)
            {
                return;
            }

            animTimer += dt * animSpeed;
            float t = Utilities.Clamp(animTimer, 0f, 1f);

            SliceLog.Log("BeforeLineVisbility");
            float revealT = Utilities.InverseLerp(0f, 0.3f, t);
            UpdateLineVisibility(Utilities.SmoothStep(topLineHiddenValue, topLineShownValue, revealT));
            UpdateLineVisibility(Utilities.SmoothStep(sideLineHiddenValue, sideLineShownValue, revealT));

            // STAGE 2: Travel (0.3 - 0.7)
            // Corners move from the center cluster to their outer edges
            //float travelT = Utilities.InverseLerp(0.3f, 0.7f, t);
            //trans.Pos_X = (int)Utilities.SmoothStep(startPosValue.x, endPosValue.x, travelT);
            //trans.Pos_Y = (int)Utilities.SmoothStep(startPosValue.y, endPosValue.y, travelT);

            // STAGE 3: Hide (0.7 - 1.0)
            // Lines disappear at the final destination
            //float hideT = Utilities.InverseLerp(0.7f, 1.0f, t);
            //if (hideT > 0)
            //{
            //    UpdateLineVisibility(Utilities.SmoothStep(topLineHiddenValue, topLineShownValue, hideT));
            //    UpdateLineVisibility(Utilities.SmoothStep(sideLineHiddenValue, sideLineShownValue, hideT));
            //    //triggerAnimation = false;

            //}

            if (t >= 1f) triggerAnimation = false;

        }

        public void UpdateLineVisibility(float val)
        {
            if (hLineTrans != null)
            {
                SetRectProperty(hLineTrans, horizontalPropName, (int)val);
            }
            if (sLineTrans != null)
            {
                sLineTrans.Bot = (int)val;
            }
        }

        public void SetRectProperty(RectTransform rect, string prop, int val)
        {
            if (prop == "Left") 
            { 
                rect.Left = val; 
            }else if (prop == "Right") 
            { 
                rect.Right = val; 
            }
        }

        public void TriggerBorderAnim()
        {
            animTimer = 0f;          // Reset the timer to the start
            triggerAnimation = true; // Allow OnUpdate to run the logic
        }
    }
}

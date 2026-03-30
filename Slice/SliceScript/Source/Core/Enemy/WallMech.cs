using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class WallMech : SliceBehaviour
    {
        GameObject topWall;
        GameObject bottomWall;

        public bool isMoving = false;
        Vector3 startPos;
        public Vector3 endPosOffset;
        public float waitingTime = 2.0f;

        public bool isSwapping = true;
        float swapTimer = 0.0f;
        public float swapInterval = 3.0f;

        // Movement state
        Vector3 targetPos;
        bool goingToEnd = true;
        float waitTimer = 0.0f;
        bool isWaiting = false;
        public float moveSpeed = 2.0f;

        bool curState = true;
        public override void OnCreate()
        {
            base.OnCreate();

            startPos = transform.Position;
            targetPos = startPos + endPosOffset;

            GameObject[] children = gameObject.GetAllChildren();
            foreach (GameObject child in children)
            {
                if (child.tag == "TopWall")
                {
                    topWall = child;
                }
                if (child.tag == "BottomWall")
                {
                    bottomWall = child;
                }
            }

            if (isSwapping)
            {
                topWall.SetActive(true);
                bottomWall.SetActive(false);
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            HandleMovement(dt);
            HandleSwapping(dt);
        }

        void HandleMovement(float dt)
        {
            if (!isMoving)
                return;

            if (isWaiting)
            {
                waitTimer += dt;
                if (waitTimer >= waitingTime)
                {
                    waitTimer = 0.0f;
                    isWaiting = false;

                    // Switch direction
                    goingToEnd = !goingToEnd;
                    targetPos = goingToEnd ? startPos + endPosOffset : startPos;
                }
                return;
            }

            Vector3 current = transform.Position;
            Vector3 toTarget = targetPos - current;
            float distance = toTarget.Magnitude();

            float step = moveSpeed * dt;

            if (step >= distance)
            {
                transform.Position = targetPos;
                isWaiting = true;
            }
            else
            {
                Vector3 direction = toTarget / distance;
                transform.Translate(direction * step);
            }
        }

        void HandleSwapping(float dt)
        {
            if (!isSwapping)
                return;

            swapTimer += dt;

            if (swapTimer >= swapInterval)
            {
                swapTimer = 0.0f;

                topWall.SetActive(!curState);
                bottomWall.SetActive(curState);

                curState = !curState;
            }
        }
    }
}
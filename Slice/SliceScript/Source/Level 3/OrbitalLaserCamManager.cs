using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class OrbitalLaserCamManager : SliceBehaviour
    {
        public bool isBeingScanned = false;
        public bool isBeingScannedLastFrame = false;
        public bool isBeingHit = false;
        public bool isHitLastFrame = false;

        public GameObject camObj;
        public Camera cam;
        Vector3 impactPos;
        Queue hits = new Queue();

        GameObject canvasObj;

        public override void OnCreate()
        {
            base.OnCreate();
            cam = camObj.GetComponent<Camera>();
        }

        public override void OnLateUpdate(float dt)
        {
            base.OnLateUpdate(dt);

            // laser interactions

            if (isBeingHit && !isHitLastFrame)
            {
                cam.SetImpactFrame(true);
                isHitLastFrame = true;
            }
            else if (!isBeingHit && isHitLastFrame)
            {
                cam.SetImpactFrame(false);
                isHitLastFrame = false;
            }

            if (isBeingHit && hits.Count != 0)
            {
                cam.SetImpactFramePosition((Vector3)hits.Dequeue());
            }

            // hint interactions

            if (canvasObj == null)
            {
                canvasObj = gameObject.FindGameObjectsWithTag("OrbitalLaserCanvasHint")[0];
            }

            if (isBeingScanned && !isBeingScannedLastFrame)
            {
                canvasObj.SetActive(true);
                isBeingScannedLastFrame = true;
            }

            else if (!isBeingScanned && isBeingScannedLastFrame)
            {
                canvasObj.SetActive(false);
                isBeingScannedLastFrame = false;
            }

            isHitLastFrame = isBeingHit;
            isBeingScannedLastFrame = isBeingScanned;

            isBeingHit = false;
            isBeingScanned = false;

            hits.Clear();
        }

        public void Hit(Vector3 pos)
        {
            hits.Enqueue(pos);
            isBeingHit = true;
        }

        public void Scanned()
        {
            isBeingScanned = true;
        }
    }
}

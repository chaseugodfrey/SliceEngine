using System.Collections;

namespace SliceEngine
{
    public class DestroyAfterDelay : SliceBehaviour
    {
        public float delay;
        public override void OnCreate()
        {
            StartCoroutine(DestroyAfterDelayCoroutine());
        }

        public IEnumerator DestroyAfterDelayCoroutine()
        {
            yield return new WaitForSeconds(delay);
            gameObject.Destroy();
        }
    }
}
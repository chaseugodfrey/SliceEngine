using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class IdleState : BaseState
    {
        public IdleState(GameObject owner) : base(owner)
        {
        }
        public override void OnEnter()
        {
            Console.WriteLine("Idle state entered");
        }

        public override void OnUpdate(float dt)
        {
            if (owner != null)
            {
                EnemyLevel2 enemy = owner.As<EnemyLevel2>();

                // update movement for idle
                enemy.IdleMovement(dt);


            }
        }
    }

    public class EnemyLevel2 : SliceBehaviour
    {
        StateMachine stateMachine;
        IdleState idleState;

        // Where it will move to when idle
        public List<GameObject> idlePoints = new List<GameObject>();
        int currPoint = 0;
        public float movementCooldown = 5.0f;
        public float movementTimer = 0.0f;
        public bool movementDone = false;

        public override void OnCreate()
        {
            // Initialize state machine and states
            stateMachine = new StateMachine();
            idleState = new IdleState(this.gameObject);

            // start at a random point first also
            currPoint = GetNextIdlePoint();


        }

        public override void OnUpdate(float dt)
        {
            stateMachine.OnUpdate(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            stateMachine.OnFixedUpdate(dt);
        }

        public int GetNextIdlePoint()
        {
            // if theres only 1 point, then itll unfortunately have to stay at 1 position
            if (idlePoints.Count == 1)
                return 0;

            // get a random point to teleport to
            int nextPoint = SliceRandom.RangeInt(0, idlePoints.Count);
            while (nextPoint != currPoint)
            {
                nextPoint = SliceRandom.RangeInt(0, idlePoints.Count);
            }

            return nextPoint;
        }

        public void IdleMovement(float dt)
        {
            if (movementDone)
                movementTimer += dt;

            if (movementTimer >= movementCooldown)
            {
                movementTimer = 0.0f;
                currPoint = GetNextIdlePoint();
                movementDone = false;
                // move to the random point
                StartCoroutine(MoveToPoint(transform.Position, idlePoints[currPoint].GetComponent<Transform>().Position, 3.0f));
            }


        }

        private IEnumerator MoveToPoint(Vector3 startPos, Vector3 targetPos, float duration)
        {
            float elapsedTime = 0.0f;

            Transform transform = this.gameObject.GetComponent<Transform>();

            while (elapsedTime < duration)
            {
                elapsedTime += Time.deltaTime;
                float t = elapsedTime / duration;
                transform.Position = Vector3.Lerp(startPos, targetPos, t);
                yield return null; // Wait for the next frame
            }

            transform.Position = targetPos; // Ensure it ends exactly at the target position
        }
    }
}

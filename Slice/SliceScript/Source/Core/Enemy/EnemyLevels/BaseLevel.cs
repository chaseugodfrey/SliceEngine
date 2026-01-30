using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{

    public class BaseLevel : SliceBehaviour
    {
        public List<GameObject> enemyPoints = new List<GameObject>();
        public GameObject levelDirector;

        // Maybe change to a list down the line to randomise
        public Prefab enemyPrefab; 
        public bool ToggleLevel = false;
        // Gap between each enemy spawning per enemy point
        public float spawnInterval = 1.0f;
        // gap between each wave after going through all points
        public float waveInterval = 2.0f;
        // keep track of which point we're spawning at now
        public int currPoint = 0;

        public float timer = 0.0f;

        public virtual bool CheckObjective() { return false; }

        public override void OnUpdate(float dt)
        {
            if (CheckObjective())
            {
                ToggleLevel = true;
            }

            if (ToggleLevel)
            {
                return;
            }

            timer += dt;

            // when it reaches the last enemy point
            if (currPoint >= enemyPoints.Count)
            {
                // use wave interval instead
                if (timer > waveInterval)
                {
                    currPoint = 0;
                    timer = 0.0f;
                }
                return;
            }

            if (levelDirector == null)
            {
                return;
            }

            if (timer > spawnInterval)
            {
               GameObject enemy =  levelDirector.As<LevelDirector>().CreateEnemy(enemyPrefab);
                enemy.GetComponent<Transform>().Position = enemyPoints[currPoint].GetComponent<Transform>().Position;

                timer = 0.0f;
                currPoint++;
            }

        }

        public virtual void EnemyKilled(GameObject enemy) { }


    }
}

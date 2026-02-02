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
        public Prefab enemyPrefab = new Prefab("Prefabs/EnemySlime.prefab"); 
        public bool ToggleLevel = false;
        // Gap between each enemy spawning per enemy point
        public float spawnInterval = 1.0f;
        // gap between each wave after going through all points
        public float waveInterval = 2.0f;
        // keep track of which point we're spawning at now
        public int currPoint = 0;

        //public int levelIndex = 0; // for debugging so I know which level is triggering

        public float timer = 0.0f;

        public virtual bool CheckObjective() { return false; }

        public override void OnUpdate(float dt)
        {
           // SliceLog.Log("In Base level " + levelIndex + "Update");

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
           //SliceLog.Log("Curr time : " + timer);

            if (timer > spawnInterval)
            {
                SliceLog.Log("CREATING ENEMY");
                Transform pointTransform = enemyPoints[currPoint].GetComponent<Transform>();
                if (!levelDirector.As<LevelDirector>().CanCreateEnemy(pointTransform.WorldPosition))
                {
                    SliceLog.Log("Died in here 3");
                    // if the curr point can't then just go next point instead
                    timer = 0.0f;
                    currPoint++;
                    return;
                }

                GameObject enemy =  levelDirector.As<LevelDirector>().CreateEnemy(enemyPrefab);
                //SliceLog.Log("Point Position = " + pointTransform.WorldPosition.x + ", " + pointTransform.WorldPosition.y + ", " + pointTransform.WorldPosition.z);
                enemy.GetComponent<Transform>().Position = pointTransform.WorldPosition;

                timer = 0.0f;
                currPoint++;
            }

        }

        public virtual void EnemyKilled(GameObject enemy) { }


    }
}

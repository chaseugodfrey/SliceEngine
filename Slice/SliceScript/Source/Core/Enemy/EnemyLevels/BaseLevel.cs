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
        public GameObject levelDirectorObject;

        public delegate void levelCompleteObserver();
        public event levelCompleteObserver LevelCompleteEvent;

        public GameObject respawnPoint;

        public int levelIndex = 0;

        // If it will constantlyy spawn once the total enemies drops below max
        public bool constantSpawning = false;
        // Keep spawning till it hits max;
        public int maxEnemies = 0;
        public int slimeCount = 0;
        private bool toggleSpawning = true;
        // Maybe change to a list down the line to randomise
        public Prefab enemyPrefab = new Prefab("Prefabs/EnemyGrunt.prefab"); 
        public bool toggleLevel = false;
        // Gap between each enemy spawning per enemy point
        public float spawnInterval = 1.0f;
        // gap between each wave after going through all points
        public float waveInterval = 2.0f;
        // keep track of which spawn point we're spawning at now
        public int currPoint = 0;

        //public int levelIndex = 0; // for debugging so I know which level is triggering

        public float timer = 0.0f;

        public virtual bool CheckObjective() { return false; }

        //public void LevelCompleteDebug()
        //{
        //    SliceLog.Log("Level complete has been called");
        //}

        //public override void OnAwake()
        //{
        //    base.OnAwake();
        //    this.LevelCompleteEvent += LevelCompleteDebug;
        //}

        public override void OnUpdate(float dt)
        {
           // SliceLog.Log("In Base level " + levelIndex + "Update");

            if (CheckObjective())
            {
                if (LevelCompleteEvent != null)
                {
                    LevelCompleteEvent();
                }
                toggleLevel = true;
            }

            //SliceLog.Log("Base Level passed check objective");

            if (toggleLevel)
            {
                return;
            }

            //SliceLog.Log("Base Level passed toggle level");

            if (toggleSpawning)
            {
                timer += dt;
            }

            //SliceLog.Log("Base Level passed toggle spawning");

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

            //SliceLog.Log("Base Level passed curr");

            if (levelDirectorObject == null)
            {
                return;
            }
            //SliceLog.Log("Curr time : " + timer);

            //SliceLog.Log("Base Level passed ;vl dri object");

            if (toggleSpawning && timer > spawnInterval)
            {
                //SliceLog.Log("CREATING ENEMY");
                Transform pointTransform = enemyPoints[currPoint].GetComponent<Transform>();
                if (!levelDirectorObject.As<LevelDirector>().CanCreateEnemy(pointTransform.WorldPosition))
                {
                    //SliceLog.Log("Died in here 3");
                    // if the curr point can't then just go next point instead
                    timer = 0.0f;
                    currPoint++;
                    return;
                }

                GameObject enemy =  levelDirectorObject.As<LevelDirector>().CreateEnemy(enemyPrefab);
                //SliceLog.Log("Point Position = " + pointTransform.WorldPosition.x + ", " + pointTransform.WorldPosition.y + ", " + pointTransform.WorldPosition.z);
                enemy.GetComponent<Transform>().Position = pointTransform.WorldPosition;

                timer = 0.0f;
                currPoint++;

                if (maxEnemies <= levelDirectorObject.As<LevelDirector>().EnemyCount())
                {
                    //SliceLog.Log("Max enemies spanwed");
                    toggleSpawning = false;
                }

            }

            //SliceLog.Log("Base Level should be working");

        }

        public virtual void EnemyKilled(GameObject enemy) 
        { 
            if (constantSpawning && maxEnemies > levelDirectorObject.As<LevelDirector>().EnemyCount())
            {
                toggleSpawning = true;
            }
        }


    }
}

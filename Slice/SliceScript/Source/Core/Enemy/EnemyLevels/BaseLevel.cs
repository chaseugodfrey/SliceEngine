using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{

    public class BaseLevel : SliceBehaviour
    {
        public List<GameObject> enemyPoints = new List<GameObject>();
        public GameObject levelDirectorObject;

        public delegate void levelCompleteObserver();
        public event levelCompleteObserver MovingToNextLevelEvent;
        public event levelCompleteObserver LevelCompleteEvent;

        public GameObject respawnPoint;

        public int levelIndex = 0;

        public bool willSpawn = false;
        // If it will constantlyy spawn once the total enemies drops below max
        public bool constantSpawning = false;
        public bool stopWhenCleared = true;
        // Keep spawning till it hits max;
        public int maxGrunts = 0;
        public int currSlimes = 0;
        public int maxSlimes = 0;
        public float slimeSpawnRate = 0f;
        public bool toggleSpawning = true;
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

        public void MoveToNextLevelDebug()
        {
            SliceLog.Log("Level complete has been called");
        }

        public override void OnCreate()
        {
            base.OnCreate();
            //SliceLog.Log("Level on create called");
            this.MovingToNextLevelEvent += MoveToNextLevelDebug;

            //if (LevelCompleteEvent.GetInvocationList() != null )
            //{
            //    SliceLog.Log("EVent null chheck passed");
            //}
            //else
            //{
            //    SliceLog.Log("EVent null chheck failed");
            //}
                
            // if (LevelCompleteEvent.GetInvocationList().Length != 0)
            //{

            //    SliceLog.Log("EVent zero count chheck passed");
            //}
            //else
            //{
            //    SliceLog.Log("EVent zero count chheck failed");
            //}
        }

        public void TriggerMovingNextLevelEvent()
        {
            if (MovingToNextLevelEvent != null)
            {
                MovingToNextLevelEvent();
            }
        }

        public void TriggerLevelCompleteEvent()
        {
            if (LevelCompleteEvent != null)
            {
                LevelCompleteEvent();

            }
        }

        public virtual void UpdateLevel(float dt)
        {
           // SliceLog.Log("In Base level " + levelIndex + "Update");

            if (CheckObjective())
            {
                toggleLevel = true;
            }

            if (toggleLevel && stopWhenCleared)
            {
                //SliceLog.Log("Level" + levelIndex + " stopped");
                return;
            }

            if (!willSpawn)
            {
                return;
            }

            // SliceLog.Log("Base Level passed toggle level");
            timer += dt;
            // SliceLog.Log("Timer : " + timer);
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
            if (timer > spawnInterval)
            {
                timer = 0.0f;
            }

            //SliceLog.Log("Base Level should be working");

        }

        public virtual void EnemyKilled(GameObject enemy) 
        { 


        }


    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class LevelDirector : SliceBehaviour, IInitializable
    {
        public List<GameObject> levels = new List<GameObject>();
        public List<GameObject> enemies = new List<GameObject>();
        public List<GameObject> levelTriggers = new List<GameObject>();
        // To prevent spawning on the same point
        public float SafetyDistance = 4.0f;
        public int currLevel = 0;
        public bool levelDone = false;

        /// <summary>
        /// Initialize the levels and stuff
        /// </summary>
        public void Initialize()
        {
            foreach(GameObject trigger in levelTriggers)
            {
                trigger.As<GeneralHitbox>().HitBoxListeners += TriggerNextLevel;
                trigger.As<GeneralHitbox>().TurnOn(); // turn on all hitboxes first, cause they'll be planned to be sequential anyway
            }
        }

        public GameObject CreateEnemy(Prefab prefab)
        {
            // instantiate the enemy
            GameObject newEnemy = prefab.Instantiate();
            enemies.Add(newEnemy);

            return newEnemy;
        }

        public void EnemyDeath(GameObject enemy)
        {
            // update the level that an enemy died
            levels[currLevel].As<BaseLevel>().EnemyKilled(enemy);

            enemies.Remove(enemy);
        }

        public override void OnUpdate(float dt)
        {
            if (currLevel > levels.Count)
            {
                SliceLog.Error("Current level is more than the number of levels");
            }
            else if (currLevel == levels.Count)
            {
                SliceLog.Log("End of level director");
                // end of level director?
                return;
            }
            
            // if the curr level is done
            if (levels[currLevel].As<BaseLevel>().CheckObjective() && !levelDone)
            {
                // then move on to next level
                levelDone = true;

                // kill all the remaining enemies
                foreach (GameObject enemy in enemies)
                {
                    enemy.Destroy();
                }

                enemies.Clear();
            }

            // if the level is done, then dont continue updating
            // the trigger box will toggle the next level
            if (levelDone)
                return;
            //SliceLog.Log("Updating Level: " + currLevel);
            levels[currLevel].As<BaseLevel>().OnUpdate(dt);
        }

        /// <summary>
        /// Check if the current enemy spawn point has room to create a nwe enemy
        /// to prevent creating an enemy ontop an enemy
        /// </summary>
        /// <param name="Pos">Pos of waypoint</param>
        /// <returns>if cannot then must can</returns>
        public bool CanCreateEnemy(Vector3 Pos)
        {
            // do simple dist check 
            foreach(GameObject enemy in enemies)
            {
                SliceLog.Log("Died in here 0");

                float Dist = (enemy.GetComponent<Transform>().WorldPosition - Pos).LengthSquared();
                SliceLog.Log("Died in here 1");
                if (Dist < SafetyDistance)
                {
                    return false;
                }
            }
            SliceLog.Log("Died in here 2");

            return true;
        }

        /// <summary>
        /// Trigger the next level to update the level obj
        /// Takes in next level to prevent double triggering or triggering the wrong level
        /// </summary>
        /// <param name="nextLevel">The next level coming</param>
        public void TriggerNextLevel(GameObject input)
        {
            SliceLog.Log("Triggering Next Level Part 1");
            // only if they done w the current level
            if (!levelDone)
                return;
            SliceLog.Log("Triggering Next Level Part 2");

            if (input.Has<PlayerController>() && Bootstrap.Player == input.As<PlayerController>())
            {
                // note: this is assuming we start at lvl 0, then on the first trigger box
                // we go to the next level

                // if we need to trigger a level box at the start then this might need a diff logic
                levelTriggers[currLevel].As<GeneralHitbox>().TurnOff();

                currLevel++;
                levelDone = false;

                SliceLog.Log("Triggering Next Level Part 3");

            }



            // only allow moving forward
            // no moving backwards/same level triggering
            // if they somehow trigger the next next level then idk i guess its fine
            //if (currLevel < nextLevel)
            //{
            //    currLevel = nextLevel;
            //    levelDone = false;
            //}
        }
    }
}

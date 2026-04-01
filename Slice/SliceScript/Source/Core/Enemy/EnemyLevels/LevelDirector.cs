using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class LevelDirector : SliceBehaviour, IInitializable
    {
        //public List<GameObject> levelsList = new List<GameObject>();
        //public Dictionary<int, GameObject> levels = new Dictionary<int, GameObject>();
        public List<GameObject> enemies = new List<GameObject>();
        public List<GameObject> levels = new List<GameObject>();
        public List<GameObject> levelTriggers = new List<GameObject>();
        //public List<GameObject> respawnPoint = new List<GameObject>();
        //public Dictionary<int, GameObject> finishedTriggers = new Dictionary<int, GameObject>();
        // To prevent spawning on the same point
        public float SafetyDistance = 4.0f;
        public int currLevel = 0;
        public bool levelDone = false;

        public GameObject deathBox;

        public Prefab enemyGruntPrefab = new Prefab("Prefabs/EnemyGrunt.prefab");
        public Prefab enemySlimePrefab = new Prefab("Prefabs/EnemySlime.prefab");

        public bool isActive = false;

        public string nextSceneToLoad = "";
        public string currSceneToLoad = "";

        /// <summary>
        /// Initialize the levels and stuff
        /// </summary>
        public void Initialize()
        {
            Cursor.state = Cursor.STATE.DISABLED;

            foreach (GameObject trigger in levelTriggers)
            {
                //Console.WriteLine("id of triggerbox: " + trigger.mID);
                trigger.As<GeneralHitbox>().HitBoxListeners += TriggerNextLevel;
                trigger.As<GeneralHitbox>().TurnOn(); // turn on all hitboxes first, cause they'll be planned to be sequential anyway
            }

            if (deathBox != null)
            {
                //SliceLog.Log("Death box is not empty, setting it");
                deathBox.As<GeneralHitbox>().HitBoxListeners += RespawnPlayer;
                deathBox.As<GeneralHitbox>().TurnOn();
            }

            //isActive = true;


            Console.WriteLine("Num of level triggers: " + levelTriggers.Count);
            Console.WriteLine("Num of levels: " + levels.Count);
        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_O))
            {
                //Vector3 loc = levels[currLevel].As<BaseLevel>().respawnPoint.GetComponent<Transform>().Position;
                //SliceLog.Log($"x: {loc.x}, y: {loc.y}, z: {loc.z}");
                //SliceLog.Log("Teleporting player");
                //Bootstrap.Player.TeleportPlayer(levels[currLevel].As<BaseLevel>().respawnPoint.GetComponent<Transform>().Position);
                RestartLevel();
            }

            if (Input.IsKeyPressed(Keys.KEY_P))
            {
                //SliceLog.Log("LEVEL DIRECTOR DEBUG TRIGGERED");
                //TriggerNextLevel(Bootstrap.Player.gameObject);
                LoadNextLevel();
            }

            if (!isActive)
                return;
            //SliceLog.Log("BBBBBBBBBBBB");
            if (currLevel > levels.Count)
            {
                //SliceLog.Error("Current level is more than the number of levels");
            }
            else if (currLevel == levels.Count)
            {
                //SliceLog.Log("End of level director");
                // end of level director?
                return;
            }

            //SliceLog.Log("aaaaaaaaa");
            // if the curr level is done
            if (levels[currLevel].As<BaseLevel>().CheckObjective() && !levelDone)
            {
                // then move on to next level
                levelDone = true;
                //SliceLog.Log("qqqqqqqqqqqq");
                // kill all the remaining enemies
                foreach (GameObject enemy in enemies)
                {
                    enemy.Destroy();
                }
                //SliceLog.Log("wwwwwwwwww");
                enemies.Clear();
            }

            // if the level is done, then dont continue updating
            // the trigger box will toggle the next level
            if (levelDone && levels[currLevel].As<BaseLevel>().stopWhenCleared)
                return;
            SliceLog.Log("Updating Level: " + currLevel);
            levels[currLevel].As<BaseLevel>().UpdateLevel(dt);
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
            foreach (GameObject enemy in enemies)
            {
                if (enemy == null) continue;

                ///SliceLog.Log("Died in here 0");
                if (enemy.mID == 0) continue;

                float Dist = (enemy.GetComponent<Transform>().WorldPosition - Pos).LengthSquared();
                //SliceLog.Log("Died in here 1");
                if (Dist < SafetyDistance)
                {
                    return false;
                }
            }
            //SliceLog.Log("Died in here 2");

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
            //SliceLog.Log("Triggering Next Level Part 2");

            //if (finishedTriggers.ContainsKey((int)input.mID))
            //    return;

            if (input.Has<PlayerController>() && Bootstrap.Player == input.As<PlayerController>())
            {
                foreach (GameObject enemy in enemies)
                {
                    enemy.Destroy();
                }

                enemies.Clear();

                // note: this is assuming we start at lvl 0, then on the first trigger box
                // we go to the next level
                levels[currLevel].As<BaseLevel>().TriggerMovingNextLevelEvent();


                // if we need to trigger a level box at the start then this might need a diff logic
                levelTriggers[currLevel].As<GeneralHitbox>().TurnOff();

                currLevel++;
                levelDone = false;
                //finishedTriggers.Add((int)input.mID, input);

                SliceLog.Log("Triggering Next Level. Curr Level:" + currLevel);

                //SliceLog.Log("Triggering Next Level Part 3");
                levels[currLevel].As<BaseLevel>().toggleLevel = false;
                levels[currLevel].As<BaseLevel>().toggleSpawning = true;

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


        public void RespawnPlayer(GameObject input)
        {
            //SliceLog.Log("Respawn is called");
            if (!input.Has<PlayerController>())
            {
                //SliceLog.Log("There is no player script in the object");
                return;
            }

            if (levels[currLevel].Has<BaseLevel>() && levels[currLevel].As<BaseLevel>().respawnPoint != null)
            {
                ////SliceLog.Log("Teleporting player");
                Bootstrap.Player.TeleportPlayer(levels[currLevel].As<BaseLevel>().respawnPoint.GetComponent<Transform>().WorldPosition);
            }

        }

        public void Lose()
        {
            Bootstrap.HUDManager.GameLoseScreen();
        }

        public void Win()
        {
            Bootstrap.HUDManager.GameWinScreen();
        }

        public void LoadNextLevel()
        {
            SceneManager.LoadScene(nextSceneToLoad);
        }

        public void RestartLevel()
        {
            SceneManager.LoadScene(currSceneToLoad);
        }
    }
}
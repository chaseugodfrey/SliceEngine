using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public static class SceneManager
    {
        private static List<Scene> _loadedScenes = new List<Scene>();
        private static Scene _activeScene;

        public static event Action<Scene> SceneLoaded;
        public static event Action<Scene> SceneUnloaded;
        public static event Action<Scene> ActiveSceneChanged;

        public static void LoadScene(string name)
        {

            //int index = _loadedScenes.Count;
            //var scene = new Scene(name, index);

            //_loadedScenes.Add(scene);
            //_activeScene = scene;

            //SceneLoaded?.Invoke(scene);
            //ActiveSceneChanged?.Invoke(scene);
            FunctionCalls.Scene_LoadScene(name);

            Console.WriteLine($"[SceneManager] Loaded scene: {name}");
        }

        public static void LoadScene(Scene scene)
        {
            //FunctionCalls.Scene_Load(scene.Name);
            _loadedScenes.Add(scene);
            _activeScene = scene;

            SceneLoaded?.Invoke(scene);
            ActiveSceneChanged?.Invoke(scene);

            Console.WriteLine($"[SceneManager] Loaded scene object: {scene.name}");
        }

        //Putting it here first cause idk where else to put it
        public static void QuitGame()
        {
            FunctionCalls.QuitGame();
        }


        //public static void UnloadScene(Scene scene)
        //{
        //    if (_loadedScenes.Remove(scene))
        //    {
        //        SceneUnloaded?.Invoke(scene);
        //    }
        //    Console.WriteLine($"[SceneManager] Unloaded scene: {scene.name}");
        //}

        public static void UnloadScene()
        {

            FunctionCalls.Scene_UnloadCurrentScene();
            Console.WriteLine($"Unload scene");
        }

        public static Scene GetActiveScene()
        {
            return _activeScene;
        }

        public static void SetActiveScene(Scene scene)
        {
            if (!_loadedScenes.Contains(scene))
                throw new InvalidOperationException("Scene must be loaded before being set active.");

            _activeScene = scene;
            ActiveSceneChanged?.Invoke(scene);

            Console.WriteLine($"[SceneManager] Active scene set to: {scene.name}");
        }

        public static void RestartScene()
        {
            if (_activeScene == null)
                return;

            string name = _activeScene.name;
            Console.WriteLine($"[SceneManager] Restarting scene: {name}");

            // FunctionCalls.Scene_Restart(name);
            SceneUnloaded?.Invoke(_activeScene);
            SceneLoaded?.Invoke(_activeScene);
        }

        public static IEnumerable<Scene> GetLoadedScenes()
        {
            return _loadedScenes;
        }

        public static Scene FindSceneByName(string name)
        {
            return _loadedScenes.Find(s => s.name == name);
        }
    }
}

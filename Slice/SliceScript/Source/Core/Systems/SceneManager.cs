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

        // -------------------------------------------------------------------
        // Scene Loading
        // -------------------------------------------------------------------

        public static void LoadScene(string name)
        {
            // Hook into engine backend call if you have one:
            // FunctionCalls.Scene_Load(name);

            // Simulate finding scene index (in real use, lookup from registry)
            int index = _loadedScenes.Count;
            var scene = new Scene(name, index);

            _loadedScenes.Add(scene);
            _activeScene = scene;

            SceneLoaded?.Invoke(scene);
            ActiveSceneChanged?.Invoke(scene);

            Console.WriteLine($"[SceneManager] Loaded scene: {name}");
        }

        public static void LoadScene(Scene scene)
        {
            // FunctionCalls.Scene_Load(scene.Name);
            _loadedScenes.Add(scene);
            _activeScene = scene;

            SceneLoaded?.Invoke(scene);
            ActiveSceneChanged?.Invoke(scene);

            Console.WriteLine($"[SceneManager] Loaded scene object: {scene.name}");
        }

        // -------------------------------------------------------------------
        // Scene Unloading
        // -------------------------------------------------------------------

        public static void UnloadScene(Scene scene)
        {
            if (_loadedScenes.Remove(scene))
            {
                SceneUnloaded?.Invoke(scene);
                Console.WriteLine($"[SceneManager] Unloaded scene: {scene.name}");
            }
        }

        // -------------------------------------------------------------------
        // Active Scene
        // -------------------------------------------------------------------

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

        // -------------------------------------------------------------------
        // Restart / Reload
        // -------------------------------------------------------------------

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

        // -------------------------------------------------------------------
        // Utilities
        // -------------------------------------------------------------------

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

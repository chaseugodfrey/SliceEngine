using System;
using System.Collections;
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

        private static SliceBehaviour _transitionRunner;
        private static SpriteRenderer _transitionRenderer;
        private const float TransitionDuration = 1.0f;

        public static event Action<Scene> SceneLoaded;
        public static event Action<Scene> SceneUnloaded;
        public static event Action<Scene> ActiveSceneChanged;

        public static void RegisterTransition(SliceBehaviour runner, SpriteRenderer renderer)
        {
            _transitionRunner = runner;
            _transitionRenderer = renderer;

            // Automatically Fade In when a new transition object registers (Scene Start)
            if (_transitionRunner != null && _transitionRenderer != null)
            {
                _transitionRunner.StartCoroutine(FadeInRoutine());
            }
        }

        public static void LoadScene(string name)
        {
            // FunctionCalls.Scene_Load(name);

            int index = _loadedScenes.Count;
            var scene = new Scene(name, index);

<<<<<<< Updated upstream
=======
            //_loadedScenes.Add(scene);
            //_activeScene = scene;

            //SceneLoaded?.Invoke(scene);
            //ActiveSceneChanged?.Invoke(scene);
            if (_transitionRunner != null && _transitionRenderer != null)
            {
                _transitionRunner.StartCoroutine(FadeOutAndLoad(name));
            }
            else
            {
                
                FunctionCalls.Scene_LoadScene(name);
            }
            

            //Console.WriteLine($"[SceneManager] Loaded scene: {name}");
        }

        public static void LoadScene(Scene scene)
        {
            //FunctionCalls.Scene_Load(scene.Name);
>>>>>>> Stashed changes
            _loadedScenes.Add(scene);
            _activeScene = scene;

            SceneLoaded?.Invoke(scene);
            ActiveSceneChanged?.Invoke(scene);

            Console.WriteLine($"[SceneManager] Loaded scene: {name}");
        }

<<<<<<< Updated upstream
        public static void LoadScene(Scene scene)
=======
        private static IEnumerator FadeInRoutine()
        {
            float elapsedTime = 0f;
            SetRectAlpha(1.0f); // Start black

            while (elapsedTime < TransitionDuration)
            {
                elapsedTime += Time.deltaTime;
                float t = Utilities.InverseLerp(0, TransitionDuration, elapsedTime);
                float alpha = Utilities.Lerp(1.0f, 0.0f, t); // 1 -> 0

                SetRectAlpha(alpha);
                yield return null;
            }
            SetRectAlpha(0.0f); // Ensure fully transparent
        }

        private static IEnumerator FadeOutAndLoad(string sceneName)
        {
            float elapsedTime = 0f;
            SetRectAlpha(0.0f); // Start transparent

            while (elapsedTime < TransitionDuration)
            {
                elapsedTime += Time.deltaTime;
                float t = Utilities.InverseLerp(0, TransitionDuration, elapsedTime);
                float alpha = Utilities.Lerp(0.0f, 1.0f, t); // 0 -> 1

                SetRectAlpha(alpha);
                yield return null;
            }
            SetRectAlpha(1.0f); // Ensure fully black

            // Now that screen is black, load the next scene
            FunctionCalls.Scene_LoadScene(sceneName);
        }

        private static void SetRectAlpha(float alpha)
        {
            if (_transitionRenderer != null)
            {
                Vector4 color = _transitionRenderer.Colour;
                color.w = alpha;
                _transitionRenderer.Colour = color;
            }
        }

        //Putting it here first cause idk where else to put it
        public static void QuitGame()
>>>>>>> Stashed changes
        {
            // FunctionCalls.Scene_Load(scene.Name);
            _loadedScenes.Add(scene);
            _activeScene = scene;

            SceneLoaded?.Invoke(scene);
            ActiveSceneChanged?.Invoke(scene);

            Console.WriteLine($"[SceneManager] Loaded scene object: {scene.name}");
        }


        public static void UnloadScene(Scene scene)
        {
            if (_loadedScenes.Remove(scene))
            {
                SceneUnloaded?.Invoke(scene);
                Console.WriteLine($"[SceneManager] Unloaded scene: {scene.name}");
            }
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

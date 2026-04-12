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
        public static SpriteRenderer _transitionRenderer;
        private static Coroutine fadeCoroutine;
        private const float TransitionDuration = 1.0f;

        public static event Action<Scene> SceneLoaded;
        public static event Action<Scene> SceneUnloaded;
        public static event Action<Scene> ActiveSceneChanged;

        public static void RegisterTransition(SliceBehaviour runner, SpriteRenderer renderer)
        {
            _transitionRunner = runner;
            _transitionRenderer = renderer;

            
            if (_transitionRunner != null && _transitionRenderer != null)
            {
                _transitionRunner.StartCoroutine(FadeInRoutine());
            }
        }
        public static void LoadWithoutTransition(string name)
        {
            FunctionCalls.Audio_StopAllSound();
            FunctionCalls.Scene_LoadScene(name);
        }

        public static void LoadScene(string name)
        {

            //int index = _loadedScenes.Count;
            //var scene = new Scene(name, index);


            //_loadedScenes.Add(scene);
            //_activeScene = scene;
            //_loadedScenes.Add(scene);
            //_activeScene = scene;

            //SceneLoaded?.Invoke(scene);
            //ActiveSceneChanged?.Invoke(scene);
            if (_transitionRunner != null && _transitionRenderer != null)
            {
                if (fadeCoroutine == null)
                {
                    fadeCoroutine = _transitionRunner.StartCoroutine(FadeOutAndLoad(name));
                }
            }
            else
            {
                // Fallback: If no transition object exists, load immediately
                FunctionCalls.Audio_StopAllSound();
                FunctionCalls.Scene_LoadScene(name);
            }


            //Console.WriteLine($"[SceneManager] Loaded scene: {name}");
        }

        public static void LoadScene(Scene scene)
        {
            //FunctionCalls.Scene_Load(scene.Name);

            _loadedScenes.Add(scene);
            _activeScene = scene;

            //SceneLoaded?.Invoke(scene);
            //ActiveSceneChanged?.Invoke(scene);
            

            //Console.WriteLine($"[SceneManager] Loaded scene: {name}");
        }

        private static IEnumerator FadeInRoutine()
        {
            float elapsedTime = 0f;
            if (_transitionRenderer != null)
            {
                _transitionRenderer.SetEnabled(true);
            }
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

            if (_transitionRenderer != null)
            {
                _transitionRenderer.SetEnabled(false);
            }
        }
        private static IEnumerator FadeOutAndLoad(string sceneName)
        {
            float elapsedTime = 0f;
            if (_transitionRenderer != null) _transitionRenderer.SetEnabled(true);
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
            fadeCoroutine = null;
            CoroutineManager.StopAllCoroutines(_transitionRunner);

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
        // public static void QuitGame()
        //public static void LoadScene(Scene scene)
        // {
        //     //FunctionCalls.Scene_Load(scene.Name);
        //     _loadedScenes.Add(scene);
        //     _activeScene = scene;

        //     SceneLoaded?.Invoke(scene);
        //     ActiveSceneChanged?.Invoke(scene);

        //     //Console.WriteLine($"[SceneManager] Loaded scene object: {scene.name}");
        // }

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
            //Console.WriteLine($"Unload scene");
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

            //Console.WriteLine($"[SceneManager] Active scene set to: {scene.name}");
        }

        public static void RestartScene()
        {
            if (_activeScene == null)
                return;

            string name = _activeScene.name;
            //Console.WriteLine($"[SceneManager] Restarting scene: {name}");

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

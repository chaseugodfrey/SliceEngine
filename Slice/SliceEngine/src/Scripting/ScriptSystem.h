/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptSystem.h

 author:   Gideon Francis

 email:       g.francis@digipen.edu

 brief:      Script System. Header file containing the function declarations for the script System.
				Initialises C# mono and cleans it up when its done. Load the mono assembly for the C# Project.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef SCRIPT_SYSTEM_H
#define SCRIPT_SYSTEM_H
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	class ScriptObject;
	class ScriptClass;
	enum class ScriptFieldType : int;
	struct ScriptEntity {};

	enum class ScriptCollisionType
	{
		CollideEnter,
		CollideStay,
		CollideExit,
		TriggerEnter,
		TriggerStay,
		TriggerExit
	};

	struct QueuedCollisionEvent
	{
		ScriptCollisionType type;
		Entity entity; // The entity with the script
		Entity other;  // The entity it hit
	};

	class ScriptSystem : public BaseSystem<ScriptEntity, Script>
	{
	public:
		ScriptSystem();
		~ScriptSystem();
		//// System Functions
		//void EntityDestroyed(Entity) override;

		/// <summary>
		/// Initializes the script system by setting up C# Mono, registering functions, components
		/// Load the entity classes
		/// Set up the initial entity class that all entities inherit from
		/// </summary>
		void Init();
		/// <summary>
		///  Clean up the C# mono files
		/// </summary>
		void CleanUp();

		/// <summary>
		/// Set up the assemblies, root domains and mono assembly
		/// </summary>
		void InitMono();

		/// <summary>
		/// Read the C# assembly file byte by byte
		/// </summary>
		/// <param name="filepath">file path to the C# assembly</param>
		/// <param name="outSize">size of the file after reading</param>
		/// <returns></returns>
		char* ReadBytes(const std::string& filepath, uint32_t* outSize);
		/// <summary>
		/// Starts the loading of C# assembly. Calls the readbyte, get the mono assembly from the image
		/// </summary>
		/// <param name="assemblyPath">File path to where C# assembly is</param>
		/// <returns>Assembly file after reading</returns>
		MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath);
		/// <summary>
		/// Loads the main app domain and core assembly image. Calls CSharpAssembly in this
		/// </summary>
		/// <param name="assemblyPath">Path to the assembly </param>
		void LoadMonoAssembly(const std::string& assemblyPath);
		/// <summary>
		/// Reloads main app domain
		/// </summary>
		/// <param name="assemblyPath">Path to the assembly </param>
		void ReloadAssembly();
		/// <summary>
		/// Used for debugging. Prints out all teh assembly types
		/// </summary>
		/// <param name="assembly">Assembly reference after loading</param>
		void PrintAssemblyTypes(MonoAssembly* assembly);
		/// <summary>
		/// Load all the entity classes and store it to be used for C# entities
		/// </summary>
		void LoadEntityClasses();
		/// <summary>
		/// For debugging. Print the mono heap size. Used when I had memory leaks
		/// </summary>
		void LogMonoHeapSize();
		/// <summary>
		/// Used to check if the entity classes contain the script being loaded
		/// </summary>
		/// <param name="scriptName">C# Script name</param>
		/// <returns>True or False</returns>
		bool HasEntityClass(std::string scriptName);

		ScriptFieldType GetScriptFieldType(MonoType* type, MonoClass** outElementClass, ScriptFieldType& containerType);

		std::shared_ptr<ScriptObject> GetScriptInstance(Entity entityID);

		void UpdateScriptVariables(Entity entity);

		void UpdateScriptComponent(Entity entity);

		void UpdateScriptPrefabComponent(Script& scriptComponent);

		//void UpdateAllPrefabScriptComponents();

		void UpdateExistingPrefabScript(Script& entity);

		void SubscribeToEvents();

		void UnsubscribeToEvents();

		void RemapGameObjectVariables(const std::unordered_map<uint32_t, uint32_t>& sceneGraph);

		/*!
		OnStart() -> Called when play button is pressed. Loop through all entities and get a reference to their scripts
		OnUpdate() -> Calls the script's update
		OnEnd() -> Clears the entity instance map for next play
		*/
		void OnStart(); // Calls the Enter function of all game objects
		void OnUpdate(float dt);
		void OnFixedUpdate(float dt);
		void UpdateScripts();
		void OnEnd();
		//void ReceiveMessage(Message* msg) override;
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		void OnEnabled(entt::registry& reg, entt::entity entity);
		void OnDisabled(entt::registry& reg, entt::entity entity);
		//Collision Events
		void OnCollideEnter(const OnCollisionEnterEvent& event);
		void OnCollideStay(const OnCollisionStayEvent& event);
		void OnCollideExit(const OnCollisionExitEvent& event);
		void OnTriggerEnter(const OnTriggerEnterEvent& event);
		void OnTriggerStay(const OnTriggerStayEvent& event);
		void OnTriggerExit(const OnTriggerExitEvent& event);
		void QueueCollision(ScriptCollisionType, Entity entity1, Entity entity2);
		void ProcessCollisionQueue();

		//button events
		void OnButtonClick(const OnButtonClickEvent& event);
		void OnButtonRelease(const OnButtonReleaseEvent& event);

		//Slider events
		void OnSliderValue(const OnSliderValueEvent& event);

		// Variables
		MonoDomain* mRootDomain;
		MonoDomain* mAppDomain;
		MonoAssembly* mCoreAssembly;
		MonoImage* mCoreAssemblyImage;

		bool AssemblyReloadPending = false;

		// Hold a reference to Entity class as it contains the constructor that all entity scripts runs to store mID
		//ScriptClass mEntityClass;
		std::shared_ptr<ScriptClass> mCoroutineManager;
		std::unique_ptr<ScriptObject> mCoroutineInstance;

		std::shared_ptr<ScriptClass> mTime;
		std::unique_ptr<ScriptObject> mTimeInstance;

		// keep track of every type of entity classes
		std::unordered_map<std::string, std::shared_ptr<ScriptClass>> mEntityClasses;
		// keep track of entity to script object
		std::unordered_map<Entity, std::shared_ptr<ScriptObject>> mEntityInstances;

		// cause I dont want to constantly loop through mEntitiesSet to pick up new entities
		// ill store new entities thats added in a vector
		// then loop this instead and pop when it loads its script properly since itll need to wait until a script is assigned
		std::vector<Entity> entityAdded;

		std::vector<QueuedCollisionEvent> mCollisionQueue;
		std::mutex mQueueLock;

	};

	extern ScriptSystem* gScriptSystem;
}

#endif
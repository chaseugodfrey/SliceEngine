/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Core.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Singleton for accessing systems, and data

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef CORE_H
#define CORE_H
//#include "Input/InputSystem.h"
//#include "AudioManager.h"
//#include "TransformSystem.h"
//#include "Graphics/ResourceManager.h"
//#include "Graphics/RenderManager.h"
//#include "Graphics/CameraSystem.h"
#include "ECS/BaseSystem.h"
#include "Singleton.h"
#include "ECS/GOFactory.h"
#include "../GLFWWindowManager.h"
//
// #include "Networking/NetworkSystem.h"

namespace SliceEngine
{
	class RenderManager;
	class ResourceManager;
	class AudioManager;
	class FramerateManager;
	class InputSystem;
	class ProjectSettingsManager;
	struct AudioSettings;
	class SceneSystem;
	class PhysicsSystem;
	class PrefabSystem;
	class LayerManager;
	struct SoundSystem;
	struct NetworkSystem;

	class Core : public Singleton<Core>
	{
	public:
		Core();
		~Core();
		// TODO: Update retrieving the name to use RTTR's 
		// need to create window system that stores the window handle

		void InitCore();

		void ExitCore();

		template<typename T>
		void InitSystem()
		{
			std::unique_ptr<T> system = std::make_unique<T>();
			system->Bind(mFactory.mRegistry);

			//std::string systemName = rttr::type::get<T>().get_name().to_string();
			std::string systemName = typeid(T).name();

			mSystems[systemName] = std::move(system);
			//mSystems.push_back(system);
		}

		template <typename T>
		T& GetSystem()
		{
			//std::string systemName = rttr::type::get<T>().get_name().to_string();
			std::string systemName = typeid(T).name();
			auto it = mSystems.find(systemName);
			if (it != mSystems.end())
			{
				if (auto* system = dynamic_cast<T*>(it->second.get()))
				{
					return *system;
				}
				//return std::dynamic_pointer_cast<T>(it->second);
			}

			// should never reach here
			assert("System does not exist!");
			throw std::runtime_error("System does not exist: " + systemName);
		}

		ResourceManager* GetResourceManager();

		InputSystem* GetInputSystem();

		SceneSystem* GetSceneSystem();

		RenderManager* GetRenderManager();

		AudioManager* GetAudioManager();

		//SoundSystem* GetSoundSystem();

		FramerateManager* GetFramerateManager();

		LayerManager* GetLayerManager();

		GLFWwindow* GetWindow();

		GLFWWindowManager* GetWindowManager();

		ProjectSettingsManager* GetProjectSettingsManager();

		void UnbindSystems();

		GOFactory mFactory;

		Registry& GetRegistry();

		NetworkSystem* GetNetwork();

		//NavigationSystem *GetNavAgent();
		struct debugMesh
		{
			uint32_t vao;
			uint32_t vbo;
			uint32_t drawCnt;
		};
		debugMesh debugNavMesh[2];
	private:
		std::unique_ptr<ResourceManager> mResource;
		std::unordered_map<std::string, std::unique_ptr<IBaseSystem>> mSystems;
		GLFWWindowManager mWindowManager;
		std::unique_ptr<InputSystem> mInputPtr; // ptr to input system. core owns it. singleton access via core
		std::unique_ptr<SceneSystem> mScenePtr;
		std::unique_ptr<RenderManager> mRender;
		std::unique_ptr<AudioManager> mAudioManager;
		std::unique_ptr<FramerateManager> mFramerateManager;
		std::unique_ptr<LayerManager> mLayerManager;
		
		std::unique_ptr<NetworkSystem> mNetwork;
		std::unique_ptr<ProjectSettingsManager> mProjectSettingsManager;
		//std::unique_ptr<AudioSettings> mAudioSettings;
	};

#define CoreInstance Core::GetInstance()
#define RegistryInstance Core::GetInstance()->GetRegistry()
#define ResourceManagerInstance Core::GetInstance()->GetResourceManager()
#define RenderManagerInstance Core::GetInstance()->GetRenderManager()
#define FactoryInstance Core::GetInstance()->mFactory
#define FramerateManagerInstance Core::GetInstance()->GetFramerateManager()

}

#endif

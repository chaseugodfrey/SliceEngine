#ifndef CORE_H
#define CORE_H
//#include "Input/InputSystem.h"
//#include "AudioManager.h"
//#include "TransformSystem.h"
//#include "Graphics/ResourceManager.h"
//#include "Graphics/RenderManager.h"
//#include "Graphics/CameraSystem.h"
#include "ECS/BaseSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Singleton.h"
#include "ECS/GOFactory.h"
#include "../GLFWWindowManager.h"
#include "Networking/NetworkSystem.h"

namespace SliceEngine
{
	class RenderManager;
	class ResourceManager;
	class FramerateManager;
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
		}

		ResourceManager* GetResourceManager();

		RenderManager* GetRenderManager();

		FramerateManager* GetFramerateManager();

		GLFWwindow* GetWindow();
		
		Registry& GetRegistry();
	
		void UnbindSystems();

		GOFactory mFactory;

		NetworkSystem* GetNetwork();


	private:
		std::unordered_map<std::string, std::unique_ptr<IBaseSystem>> mSystems;
		GLFWWindowManager mWindowManager;
		std::unique_ptr<ResourceManager> mResource;
		std::unique_ptr<RenderManager> mRender;
		std::unique_ptr<FramerateManager> mFramerateManager;
		std::unique_ptr<NetworkSystem> mNetwork;
	};

#define CoreInstance Core::GetInstance()
#define RegistryInstance Core::GetInstance()->GetRegistry()
#define ResourceManagerInstance Core::GetInstance()->GetResourceManager()
#define RenderManagerInstance Core::GetInstance()->GetRenderManager()
#define FactoryInstance Core::GetInstance()->mFactory
#define FramerateManagerInstance Core::Getnstance()->GetFraterateManager()

}

#endif


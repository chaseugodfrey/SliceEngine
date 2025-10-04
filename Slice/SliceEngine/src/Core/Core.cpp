#include "pch.h"
#include "Core.h"
//#include "Graphics/ResourceManager.h"
#include "Resource/ResourceManager.h"

#include "Graphics/RenderManager.h"
#include "Systems/FramerateManager.h"
#include "../AudioManager.h"
#include "Input/InputSystem.h"
namespace SliceEngine
{
	Core::Core()
	{

	}

	Core::~Core()
	{

	}

	Registry& Core::GetRegistry()
	{
		return mFactory.mRegistry;
	}

	void Core::InitCore()
	{
		mWindowManager.CreateWindow(); // This has to be first

		mResource = std::make_unique<ResourceManager>();
		mRender = std::make_unique<RenderManager>();
		mAudioManager = std::make_unique<AudioManager>();
		mFramerateManager = std::make_unique<FramerateManager>();
		//mFactory.RegisterSerializableComponent<Transform>();
		mInputPtr = std::make_unique<InputSystem>();
		mInputPtr->Init(mWindowManager.GetWindow());
		mInputPtr->BindCallbacksToWindow(mWindowManager.GetWindow());
		mFactory.RegisterComponent<Transform>();
		mFactory.RegisterComponent<SceneGraph>();
		mFactory.RegisterComponent<Renderer>();
		mFactory.RegisterComponent<Camera>();
		mFactory.RegisterComponent<SliceEntity>();
		mFactory.RegisterComponent<Script>();
		mFactory.RegisterComponent<RigidBody>();
		mFactory.RegisterComponent<ColliderShape>();
		mFactory.RegisterComponent< AudioSource>();

        mResource->InitResourceManager();
	}

	RTTR_REGISTRATION
	{
        // Shifting here because SLICE_RTTR is becoming too big of an obj file
	rttr::registration::class_<Renderer>(typeid(Renderer).name())
		.constructor<>()
		.property("model", &Renderer::model)
		.property("texture", &Renderer::texture)
		.property("renderTag", &Renderer::renderTag);
	rttr::registration::class_<Camera>(typeid(Camera).name())
		.constructor<>()
		.property("width", &Camera::width)
		.property("height", &Camera::height)
		.property("pov", &Camera::pov)
		.property("near", &Camera::near)
		.property("far", &Camera::far)
		.property("textureID", &Camera::textureID)
		.property("depthTex", &Camera::depthTex)
		.property("renderTag", &Camera::renderTag);
	rttr::registration::class_<Script>(typeid(Script).name())
		.constructor<>()
		.property("scriptName", &Script::scriptName);

	}

	void Core::ExitCore()
	{
		mWindowManager.CloseWindow();
		UnbindSystems();
	}

	void Core::UnbindSystems()
	{
		for (auto& system : mSystems)
		{
			system.second->Unbind();
			//system->Unbind();
		}

	}

	InputSystem* Core::GetInputSystem()
	{
		return mInputPtr.get();
	}

	ResourceManager* Core::GetResourceManager()
	{
		return mResource.get();
	}

	RenderManager* Core::GetRenderManager()
	{
		return mRender.get();
	}

	FramerateManager* Core::GetFramerateManager()
	{
		return mFramerateManager.get();
	}

	AudioManager* Core::GetAudioManager()
	{
		return mAudioManager.get();
	}

	GLFWwindow* Core::GetWindow()
	{

		return mWindowManager.GetWindow();
	}
}
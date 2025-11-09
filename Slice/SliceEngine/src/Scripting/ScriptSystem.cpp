/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptSystem.cpp

 author:   Gideon Francis

 email:       g.francis@digipen.edu

 brief:      Script System. cpp file containing the function definitions for the script System.
                Initialises C# mono and cleans it up when its done. Load the mono assembly for the C# Project. Holds a map
                of all the entity classes and the instances of entities

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


#include "pch.h"
#include "ScriptSystem.h"
#include <filesystem>
#include <mono/metadata/mono-gc.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include "ScriptFunctions.h"
#include <filesystem>
#include <fstream>
#include "../Core/Core.h"
#include "../Input/InputSystem.h"
#include "../Systems/SceneSystem.h"
namespace SliceEngine
{
    ScriptSystem* gScriptSystem = NULL;
    namespace
    {

        // TODO: Change this to a global config setting for engine
        static bool debug = false;
    }
    
    static std::unordered_map<std::string, ScriptFieldType> sFieldTypeMap =
    {
        {"System.Single", ScriptFieldType::Float},
        {"System.Double", ScriptFieldType::Double},
        {"System.Boolean", ScriptFieldType::Bool},
        {"System.Char", ScriptFieldType::Char},
        {"System.Int16", ScriptFieldType::Short},
        {"System.Int32", ScriptFieldType::Int},
        {"System.UInt32", ScriptFieldType::UInt},
        {"System.String", ScriptFieldType::String},
        {"SliceEngine.Vector2", ScriptFieldType::Vector2},
        {"SliceEngine.Vector3", ScriptFieldType::Vector3},
        {"SliceEngine.GameObject", ScriptFieldType::GameObject},
        {"SliceEngine.Audio", ScriptFieldType::Audio},
        {"SliceEngine.Prefab", ScriptFieldType::Prefab}
    };

    ScriptSystem::ScriptSystem()
    {
        //mSignature.set(ComponentManager::GetInstance()->GetComponentID<Script>());

        //SystemManager::GetInstance()->SetSignature<ScriptSystem>(mSignature);

        if (gScriptSystem == NULL)
            gScriptSystem = this;

        mRootDomain = nullptr;
        mAppDomain = nullptr;
        mCoreAssembly = nullptr;
        mCoreAssemblyImage = nullptr;
        //mEntityClass = nullptr;
    }

    ScriptSystem::~ScriptSystem()
    {
        // CleanUp();
    }

    void ScriptSystem::Init()
    {
       InitMono();

       ScriptFunctions::RegisterFunctions();

       LoadEntityClasses();

       ScriptFunctions::RegisterComponents();

        // PrintAssemblyTypes(mCoreAssembly);
        // retrieve the main Entity class
        mEntityClass = ScriptClass("SliceEngine", "SliceBehaviour");
        mCoroutineManager = std::make_shared<ScriptClass>("SliceEngine", "CoroutineManager");
        mCoroutineManager->Instantiate();
        mCoroutineInstance = std::make_unique<ScriptObject>(mCoroutineManager, static_cast<Entity>(0));
        SLICE_LOG("mCoroutine");

        SubscribeToEvents();
    }

    void ScriptSystem::LogMonoHeapSize()
    {
        // Retrieve the current size of the managed heap in bytes
        size_t heapSize = mono_gc_get_heap_size();

        // Convert to KB or MB if needed
        std::cout << "Current Mono managed heap size: " << heapSize << " bytes ("
            << heapSize / 1024 << " KB)" << std::endl;
    }

    void ScriptSystem::CleanUp()
    {
        if (mAppDomain)
        {
            // Switch back to root domain to allow unloading
            mono_domain_set(mRootDomain, false);

            // Unload the application domain
            mono_domain_unload(mAppDomain);
            mAppDomain = nullptr;
        }

        // Force garbage collection and wait for finalizers in managed code
        //mono_gc_collect(mono_gc_max_generation());
        // LogMonoHeapSize();

        if (mRootDomain)
        {
            // Perform JIT cleanup on the root domain
            mono_jit_cleanup(mRootDomain);
            mRootDomain = nullptr;
        }


        mCoreAssemblyImage = nullptr;  // Clear any references to images, assemblies
        mCoreAssembly = nullptr;


        // Optional: small delay to ensure Mono completes cleanup
       // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void ScriptSystem::InitMono()
    {
        // while (true) {};
        mono_set_assemblies_path("thirdparty/Mono/bin");

        //mRootDomain = rootDomain;
        if (debug)
        {
            const char* argv[2] = {
                "--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=logs/MonoDebugger.log",
                "--soft-breakpoints"
            };

            mono_jit_parse_options(2, (char**)argv);
            mono_debug_init(MONO_DEBUG_FORMAT_MONO);

            // mono_debug_domain_create(mRootDomain);
        }


        mRootDomain = mono_jit_init("SliceJITRuntime");
        if (mRootDomain == nullptr)
        {
            SLICE_LOG_ERROR("Unable to init mono");
            assert("Failed to init mono jit");
            return;
        }

        if (debug)
        {
            mono_debug_domain_create(mRootDomain);
        }


        LoadMonoAssembly("../SliceScript/SliceScript.dll");


		PrintAssemblyTypes(mCoreAssembly);
        //MonoImage* image = mono_assembly_get_image(mCoreAssembly);
        //MonoClass* monoClass = mono_class_from_name(image, "Carmicah", "Main");
        //MonoObject* classInstance = mono_object_new(mAppDomain, monoClass);

        //mono_runtime_object_init(classInstance);
    }

    /// <summary>
    /// Load a file into an array of bytes that can be passed to Mono directly
    /// </summary>
    /// <param name="filepath">File path to read from</param>
    /// <param name="outSize"> output file</param>
    /// <returns></returns>
    char* ScriptSystem::ReadBytes(const std::string& filepath, uint32_t* outSize)
    {
        std::filesystem::path directoryPath = filepath;
        if (std::filesystem::is_regular_file(directoryPath))
        {
            SLICE_LOG_DEBUG("Test");
        }

        std::ifstream stream(directoryPath, std::ios::binary | std::ios::ate);

        if (!stream)
        {
            // Failed to open the file
           // while (true) {};
            return nullptr;
        }
        // while (true) {};

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        uint32_t size = (uint32_t)(end - stream.tellg());

        if (size == 0)
        {
            // File is empty
            return nullptr;
        }

        char* buffer = new char[size];
        stream.read((char*)buffer, size);
        stream.close();

        *outSize = size;
        return buffer;
    }

    void ScriptSystem::LoadMonoAssembly(const std::string& assemblyPath)
    {
        // Create an app domain
        mAppDomain = mono_domain_create_appdomain(const_cast<char*>("SliceEngineAppDomain"), nullptr);
        mono_domain_set(mAppDomain, true);

        if (debug)
        {
            mono_debug_domain_create(mAppDomain);
        }
        mCoreAssembly = LoadCSharpAssembly(assemblyPath);
        if (mCoreAssembly == nullptr)
        {
            // assert smth here HasEntityClass
            SLICE_LOG_ERROR("Unable to load core assembly");
            assert("Unable to load");
        }

        mCoreAssemblyImage = mono_assembly_get_image(mCoreAssembly);
        if (mCoreAssemblyImage == nullptr)
        {
            // assert smth here 
            SLICE_LOG_ERROR("Unable to load core assembly image");
            assert("Unable to load");
        }



        AssemblyReloadPending = false;
    }

    void ScriptSystem::ReloadAssembly()
    {
        // temporary until we find a btr way
        // cause itll freeze the engine for a bit
        // mayb a pop up window to show its recompiling or smth by having this threaded

        int buildResult = system("dotnet build \"../SliceScript/SliceScript.csproj\"");

        if (buildResult != 0)
        {
            return;
        }

        for (auto [entity, instance] : mEntityInstances)
        {
            entityAdded.push_back(entity);
        }

        for (auto& it : mEntityInstances)
        {
            mono_gchandle_free(it.second->mHandle);
        }

        mEntityInstances.clear();
        mono_domain_set(mono_get_root_domain(), false);

        if (mAppDomain)
        {
            mono_domain_unload(mAppDomain);
            mAppDomain = nullptr;
        }

        LoadMonoAssembly("../SliceScript/SliceScript.dll");

        //ScriptFunctions::RegisterFunctions();
        LoadEntityClasses();

        ScriptFunctions::RegisterComponents();

        mEntityClass = ScriptClass("SliceEngine", "SliceBehaviour");

        mCoroutineManager = std::make_shared<ScriptClass>("SliceEngine", "CoroutineManager");
        mCoroutineManager->Instantiate();
        mCoroutineInstance = std::make_unique<ScriptObject>(mCoroutineManager, static_cast<Entity>(0));
        SLICE_LOG("mCorout");
        //PrintAssemblyTypes(mCoreAssembly);
    }


    MonoAssembly* ScriptSystem::LoadCSharpAssembly(const std::string& assemblyPath)
    {
        uint32_t fileSize = 0;
        char* fileData = ReadBytes(assemblyPath, &fileSize);

        // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

        if (status != MONO_IMAGE_OK)
        {
            // Don't forget to free the file data
            delete[] fileData;

            const char* errorMessage = mono_image_strerror(status);
            SLICE_LOG_ERROR(errorMessage);
            assert(errorMessage);
            // Log some error message using the errorMessage data
            return nullptr;
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);

        MonoImage* assemblyImage = mono_assembly_get_image(assembly);

        if (debug)
        {
            std::filesystem::path pdbPath = assemblyPath;
            pdbPath.replace_extension(".pdb");
			std::string msg = "Attempting to load pdb: {}" + pdbPath.string();
            SLICE_LOG_DEBUG(msg);

            if (std::filesystem::exists(pdbPath))
            {
                uint32_t pdbFileSize = 0;
                char* pdbFileData = ReadBytes(pdbPath.string(), &pdbFileSize);
                mono_debug_open_image_from_memory(assemblyImage, (const mono_byte*)pdbFileData, pdbFileSize);

                delete[] pdbFileData;
            }
        }

        mono_image_close(image);

        // Don't forget to free the file data
        delete[] fileData;

        return assembly;

        // return nullptr;
    }

    void ScriptSystem::PrintAssemblyTypes(MonoAssembly* assembly)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
           // UNUSED(name);
           // UNUSED(nameSpace);
           printf("%s.%s\n", nameSpace, name);
        }
    }

    bool ScriptSystem::HasEntityClass(std::string scriptName)
    {
        return mEntityClasses.find(scriptName) != mEntityClasses.end();
    }

    void ScriptSystem::OnStart()
    {
        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            scriptRef->InvokeOnConstruct((unsigned int)id);
            scriptRef->InvokeOnCreate();
        }
    }

    void ScriptSystem::OnUpdate(float dt)
    {
        mCoroutineInstance->InvokeOnUpdate(dt);

        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            scriptRef->InvokeOnUpdate(dt);
        }     
    }

    void ScriptSystem::OnFixedUpdate(float dt)
    {
        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            scriptRef->InvokeOnFixedUpdate(dt);
        }
    }

    void ScriptSystem::UpdateScripts()
    {
        for (auto entity = entityAdded.begin(); entity != entityAdded.end(); ++entity)
        {
            if (mEntityInstances.count(*entity) == 0)
            {
                auto& scriptComponent = mRegistry->get<Script>(*entity);//ComponentManager::GetInstance()->GetComponent<Script>(*entity);
                if (HasEntityClass(scriptComponent.scriptName)) // Technically dont have to check IMGUI only allows for entity classes to be picked
                {
                    std::shared_ptr<ScriptObject> scriptObj = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], *entity);
                    //  scriptRef->SetUpEntity(id); // Instantiate and set up the method handling
                   // CM_CORE_INFO("Setting up a new script");

                    mEntityInstances[*entity] = scriptObj;

                    //auto inputs = Core::GetInstance()->GetInputSystem();
                    auto scene = Core::GetInstance()->GetSceneSystem();

                    //if (inputs->GetMode() == InputMode::Game)
                    //{
                    //    //check if its running or in edit mode but for now just call
                    //    mEntityInstances[*entity]->InvokeOnConstruct((unsigned int)*entity);
                    //    mEntityInstances[*entity]->InvokeOnCreate();
                    //}

                    //if (scene->mCurrentState == SceneState::PLAY_SCENE)
                    //{
                    //    mEntityInstances[*entity]->InvokeOnConstruct((unsigned int)*entity);
                    //    mEntityInstances[*entity]->InvokeOnCreate();
                    //}

                    UpdateScriptComponent(*entity);
                    entityAdded.erase(entity);
                    break;
                    // entity = entityAdded.begin();
                }
            }
        }
    }

    void ScriptSystem::OnEnd()
    {
        for (auto& it : mEntityInstances)
        {
            mono_gchandle_free(it.second->mHandle);
        }

        mEntityInstances.clear();
        entityAdded.clear();
    }

    void ScriptSystem::UpdateScriptVariables(Entity entity)
    {
		auto& scriptComponent = mRegistry->get<Script>(entity);

        auto& scriptRef = mEntityInstances[entity];
        const auto& fields = scriptRef->GetScriptClass()->mFields;
        for (const auto& it : fields)
        {
            if (scriptComponent.scriptableFieldMap.count(it.first) != 0)
            {
                if (it.second.mType == ScriptFieldType::String)
                {
                    std::string str = scriptComponent.scriptableFieldMap[it.first].get_value<std::string>();
                    scriptRef->SetFieldValue<std::string>(it.second.mName, str);
                }
                else
                {
                    scriptRef->SetFieldValue(it.second.mName.c_str(), scriptComponent.scriptableFieldMap[it.first]);
                }
            }

        }

    }

    void ScriptSystem::UpdateScriptComponent(Entity entity)
    {
        Script& scriptComponent = mRegistry->get<Script>(entity);

        // if it has script instances attached to this entity
        if (mEntityInstances.count(entity) > 0)
        {
            auto& scriptRef = mEntityInstances[entity];
            const auto& fields = scriptRef->GetScriptClass()->mFields;
            scriptComponent.scriptableFieldMap.clear();

            for (const auto& it : fields)
            {
                if (it.second.mType == ScriptFieldType::Float)
                {
                    float var = scriptRef->GetFieldValue<float>(it.second.mName);
                    scriptComponent.scriptableFieldMap[it.first] = var;
                }
                else if (it.second.mType == ScriptFieldType::Bool)
                {
                    bool var = scriptRef->GetFieldValue<bool>(it.second.mName);
                    scriptComponent.scriptableFieldMap[it.first] = var;
                }
                else if (it.second.mType == ScriptFieldType::String)
                {
                    std::string var = scriptRef->GetFieldValue<std::string>(it.second.mName);
                    scriptComponent.scriptableFieldMap[it.first] = var;
                }
                else if (it.second.mType == ScriptFieldType::Int)
                {
                    int var = scriptRef->GetFieldValue<int>(it.second.mName);
                    scriptComponent.scriptableFieldMap[it.first] = var;
                }
            }
        }
    }

    void ScriptSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
    {
        if (mEntityInstances.count(entity) != 0)
        {
            // already has an instance
            return;
		}

		auto& scriptComponent = reg.get<Script>(entity);
        if (HasEntityClass(scriptComponent.scriptName))
        {

			std::shared_ptr<ScriptObject> instance = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
			mEntityInstances[entity] = instance;
            
            // Update the variables in script instance with variables 
            // in the script component
            UpdateScriptVariables(entity);
            // idk incase it isnt populated the first time
            UpdateScriptComponent(entity);
            // Check if an entity is created on runtime
			// if it is then we have to invoke the construct and oncreate
            // but again after M1 

            // for now we just invoke the moment it has been added
            if (Core::GetInstance()->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE)
            {
			    mEntityInstances[entity]->InvokeOnConstruct((unsigned int)entity);
			    mEntityInstances[entity]->InvokeOnCreate();

            }
		}
        else
        {
			// Script not assigned yet, so add to the entity added list
            // to check later
            entityAdded.push_back(entity);
        }
    }

    /// <summary>
    /// // If entity has it's scripting component removed 
    /// </summary>
    /// <param name="entity">Entity being removed</param>
    void ScriptSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
    {
        for (auto& it : mEntityInstances)
        {
            if (it.first == entity)
            {
                mono_gchandle_free(it.second->mHandle);

                mEntityInstances.erase(it.first);
                break;
            }
        }

        for (auto it = entityAdded.begin(); it != entityAdded.end(); ++it)
        {
            if (*it == entity)
            {
                entityAdded.erase(it);
                break;
            }
        }
    }

    void ScriptSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
    {
        
	}

        void ScriptSystem::LoadEntityClasses()
        {
            //loook here aloy

            // clear the map before using it
            mEntityClasses.clear();

            MonoImage* image = mono_assembly_get_image(mCoreAssembly);
            const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
            int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
            MonoClass* entityClass = mono_class_from_name(image, "SliceEngine", "SliceBehaviour");
            //MonoClass* testClass = mono_class_from_name(image, "SliceEngine", "CoroutineManager");
            for (int32_t i = 0; i < numTypes; i++)
            {               

                uint32_t cols[MONO_TYPEDEF_SIZE];
                mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

                const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
                const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

                // To protect against compiler generated types
                if (name[0] == '<')
                    continue;

                // To protect against private nested classes being embroiled in this
                uint32_t flags = cols[MONO_TYPEDEF_FLAGS];
                uint32_t visibility = flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;

                // Skip compiler-generated or nested types (names starting with '<')
                if (name[0] == '<')
                    continue;

                // Skip all nested types (nested types have visibility values 0x02–0x06)
                if (visibility >= TYPE_ATTRIBUTE_NESTED_PUBLIC && visibility <= TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM)
                    continue;

                MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

                // don't reload entity class
                if (monoClass == entityClass) continue;

                std::string className;
                if (strlen(nameSpace))
                {
                    className = std::format("{}.{}", nameSpace, name);
                }
                else
                    className = name;

                bool isEntityScript = mono_class_is_subclass_of(monoClass, entityClass, false);
                if (isEntityScript)
                {
                    std::shared_ptr<ScriptClass> script = std::make_shared<ScriptClass>(nameSpace, name);
                    mEntityClasses[className] = script;

                    MonoClass* currentClass = monoClass;
                    while (currentClass)
                    {
                        // get all the fields from the c# script (i.e variables from c# script side)
                        void* iterator = nullptr;
                        while (MonoClassField* field = mono_class_get_fields(currentClass, &iterator))
                        {
                            std::string fieldName = mono_field_get_name(field);
                            // Only access public variables from the mono class
                            if (mono_field_get_flags(field) & FIELD_ATTRIBUTE_PUBLIC)
                            {
                                MonoType* type = mono_field_get_type(field);
                                ScriptFieldType fieldType = GetScriptFieldType(type);


                                rttr::variant var;
                                // Store it in the script's field map
                                script->mFields[fieldName] = { fieldType, fieldName, field, var };
                            }
                        }

                        currentClass = mono_class_get_parent(currentClass);
                        if (currentClass == entityClass)
                            break;
                    }
                }

                //printf("%s.%s\n", nameSpace, name);


            }
    }

    ScriptFieldType ScriptSystem::GetScriptFieldType(MonoType* type)
    {
        std::string name = mono_type_get_name(type);
        // If the name exist in our field type map
        if (sFieldTypeMap.count(name) != 0)
        {
            auto iter = sFieldTypeMap.find(name);
            return iter->second;
        }

        return ScriptFieldType::None;
    }

    std::shared_ptr<ScriptObject> ScriptSystem::GetScriptInstance(Entity entityID)
    {
        if (mEntityInstances.count(entityID) == 0)
        {
            return nullptr;
        }

        return mEntityInstances[entityID];
    }

    void ScriptSystem::SubscribeToEvents()
    {
        auto* eventManager = EventManager::GetInstance();

        eventManager->Subscribe<OnCollisionEnterEvent, &ScriptSystem::OnCollideEnter>(this);

        eventManager->Subscribe<OnCollisionStayEvent, &ScriptSystem::OnCollideStay>(this);

        eventManager->Subscribe<OnCollisionExitEvent, &ScriptSystem::OnCollideExit>(this);

        eventManager->Subscribe<OnTriggerEnterEvent, &ScriptSystem::OnTriggerEnter>(this);

        eventManager->Subscribe<OnTriggerStayEvent, &ScriptSystem::OnTriggerStay>(this);

        eventManager->Subscribe<OnTriggerExitEvent, &ScriptSystem::OnTriggerExit>(this);

    }

    void ScriptSystem::OnCollideEnter(const OnCollisionEnterEvent& event)
    {
        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            
        //    std::cout << "On collide being called for " << (uint32_t)event.other << std::endl;
            scriptInstance->InvokeOnCollideEnter((unsigned int)event.other);
		}
    }
    void ScriptSystem::OnCollideStay(const OnCollisionStayEvent& event)
    {
        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeOnCollideStay((unsigned int)event.other);
        }
	}
    void ScriptSystem::OnCollideExit(const OnCollisionExitEvent& event)
    {
        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeOnCollideExit((unsigned int)event.other);
		}
    }
    void ScriptSystem::OnTriggerEnter(const OnTriggerEnterEvent& event)
    {
        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeOnTriggerEnter((unsigned int)event.other);
        }

    }
    void ScriptSystem::OnTriggerStay(const OnTriggerStayEvent& event)
    {
        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeOnTriggerStay((unsigned int)event.other);
        }
	}
    void ScriptSystem::OnTriggerExit(const OnTriggerExitEvent& event)
    {
        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeOnTriggerExit((unsigned int)event.other);
        }
    }
}
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
#include "ScriptObject.h"
#include <filesystem>
#include <mono/metadata/mono-gc.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/class.h>
#include "ScriptFunctions.h"
#include <filesystem>
#include <fstream>
#include "../Core/Core.h"
#include "../Input/InputSystem.h"
#include "../Systems/SceneSystem.h"
#include <shared_mutex>

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
        //mEntityClass = ScriptClass("SliceEngine", "SliceBehaviour");
        mCoroutineManager = std::make_shared<ScriptClass>("SliceEngine", "CoroutineManager");
        mCoroutineManager->Instantiate();
        mCoroutineInstance = std::make_unique<ScriptObject>(mCoroutineManager, static_cast<Entity>(0));
        SLICE_LOG("C# Coroutine System Initialized");

        mTime = std::make_shared<ScriptClass>("SliceEngine", "Time");
        mTime->Instantiate();
        mTimeInstance = std::make_unique<ScriptObject>(mTime, static_cast<Entity>(0));
        SLICE_LOG("C# Time System Initialized");

        SubscribeToEvents();

        mRegistry->on_construct<InactiveEntity>().connect<&ScriptSystem::OnDisabled>(this);
        mRegistry->on_destroy<InactiveEntity>().connect<&ScriptSystem::OnEnabled>(this);
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
        mono_gchandle_free(mCoroutineInstance->mHandle);
        mono_gchandle_free(mTimeInstance->mHandle);

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
        std::string assemblyBinPath = "Data/thirdparty/Mono/bin";

        if (!std::filesystem::exists(assemblyBinPath))
        {
            // Fallback for Editor / Dev Environment
            assemblyBinPath = "thirdparty/Mono/bin";
        }

        mono_set_assemblies_path(assemblyBinPath.c_str());

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

        std::string assemblyPath = "Data/SliceScript.dll"; // Path for Game Build

        if (!std::filesystem::exists(assemblyPath))
        {
            // Fallback for Editor / Dev Environment
            assemblyPath = "../SliceScript/SliceScript.dll";
        }


        LoadMonoAssembly(assemblyPath);


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


        // save the current script variables
        for (auto [entity, instance] : mEntityInstances)
        {
            UpdateScriptComponent(entity);
        }

        // temporary until we find a btr way
        // cause itll freeze the engine for a bit
        // mayb a pop up window to show its recompiling or smth by having this threaded
        int buildResult = system("dotnet build \"../SliceScript/SliceScript.csproj\"");
        UnsubscribeToEvents();
        if (buildResult != 0)
        {
            return;
        }

        ClearManagedHandles();

        // clear the collision queue events 
        {
            std::lock_guard<std::mutex> lock(mQueueLock);
            mCollisionQueue.clear();
        }

        for (auto [entity, instance] : mEntityInstances)
        {
            entityAdded.push_back(entity);
        }

        for (auto& it : mEntityInstances)
        {
            mono_gchandle_free(it.second->mHandle);
            it.second->mHandle = 0;
        }

        mono_gchandle_free(mCoroutineInstance->mHandle);
        mono_gchandle_free(mTimeInstance->mHandle);

        mCoroutineInstance.reset();
        mTimeInstance.reset();
        mEntityInstances.clear();
        mono_domain_set(mono_get_root_domain(), false);

        if (mAppDomain)
        {
            mono_domain_unload(mAppDomain);
            mAppDomain = nullptr;
        }

        std::string assemblyPath = "Data/SliceScript.dll"; // Path for Game Build

        if (!std::filesystem::exists(assemblyPath))
        {
            // Fallback for Editor / Dev Environment
            assemblyPath = "../SliceScript/SliceScript.dll";
        }

        LoadMonoAssembly(assemblyPath);

        //LoadMonoAssembly("../SliceScript/SliceScript.dll");

        //ScriptFunctions::RegisterFunctions();
        LoadEntityClasses();

        // one issue i foresee is if they modify a variable starting value
        // like if they default initialize a list with 1 element in it
        // then when I update with the previously saved value in the script component
        // it might overwrite 

        ScriptFunctions::RegisterComponents();


        mCoroutineManager = std::make_shared<ScriptClass>("SliceEngine", "CoroutineManager");
        mCoroutineManager->Instantiate();
        mCoroutineInstance = std::make_unique<ScriptObject>(mCoroutineManager, static_cast<Entity>(0));

        mTime = std::make_shared<ScriptClass>("SliceEngine", "Time");
        mTime->Instantiate();
        mTimeInstance = std::make_unique<ScriptObject>(mTime, static_cast<Entity>(0));

        for (auto entity = entityAdded.begin(); entity != entityAdded.end(); ++entity)
        {
            if (mEntityInstances.count(*entity) == 0)
            {
                auto& scriptComponent = mRegistry->get<Script>(*entity);//ComponentManager::GetInstance()->GetComponent<Script>(*entity);
                if (HasEntityClass(scriptComponent.scriptName)) // Technically dont have to check IMGUI only allows for entity classes to be picked
                {
                    std::shared_ptr<ScriptObject> scriptObj = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], *entity);
                    //  scriptRef->SetUpEntity(id); // Instantiate and set up the method handling

                    mEntityInstances[*entity] = scriptObj;

                    UpdateScriptVariables(*entity);

                    // entity = entityAdded.begin();
                }
            }
        }

        entityAdded.clear();
        
        SubscribeToEvents();

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
        mCoroutineInstance->InvokeOnCreate();
        mTimeInstance->InvokeOnCreate();

        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            //continue if disabled
            auto& scriptComponent = mRegistry->get<Script>(id);
            if (!scriptComponent.componentEnabled)
                continue;

            if (entityConstructed.contains(id))
				continue;

            scriptRef->InvokeOnConstruct((unsigned int)id);
        }

        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            //continue if disabled
            auto& scriptComponent = mRegistry->get<Script>(id);
            if (!scriptComponent.componentEnabled)
                continue;

            if (entityConstructed.contains(id))
                continue;

            scriptRef->InvokeOnAwake();
        }


        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            //continue if disabled
            auto& scriptComponent = mRegistry->get<Script>(id);
            if (!scriptComponent.componentEnabled)
                continue;

            if (entityConstructed.contains(id))
                continue;

            scriptRef->InvokeOnCreate();
            UpdateScriptComponent(id);
        }

        //for (const auto& id : entityConstructed)
        //{
        //    mEntityInstances[id]->InvokeOnAwake();
        //    mEntityInstances[id]->InvokeOnCreate();

        //}

        entityConstructed.clear();
    }

    void ScriptSystem::OnUpdate(float dt)
    {
        ProcessCollisionQueue();

        mCoroutineInstance->InvokeOnUpdate(dt);
        mTimeInstance->InvokeOnUpdate(dt);

        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            std::string entityName = FactoryInstance.GetGOByEntity(id).GetName();
            auto& scriptComponent = mRegistry->get<Script>(id);

            //if disabled should not update
            if (!scriptComponent.componentEnabled)
            {
                continue;
            }

            if (scriptRef == nullptr)
            {
                SLICE_LOG_ERROR("Error in initializing script reference");
            }
            else
            {
                scriptRef->InvokeOnUpdate(dt);
                UpdateScriptComponent(id);
            }
        }
    
        for (const auto& [id, entitySet] : mCollideMap)
        {
            auto scriptInstance = mEntityInstances[id];

            auto& scriptComponent = mRegistry->get<Script>(id);

            //if disabled should not update
            if (!scriptComponent.componentEnabled)
            {
                continue;
            }

            for (const auto& ent : entitySet)
            {
                 scriptInstance->InvokeOnCollideStay((unsigned int)ent);
            }
        }

        for (const auto& [id, entitySet] : mTriggerMap)
        {
            auto scriptInstance = mEntityInstances[id];

            auto& scriptComponent = mRegistry->get<Script>(id);

            //if disabled should not update
            if (!scriptComponent.componentEnabled)
            {
                continue;
            }

            for (const auto& ent : entitySet)
            {
                 scriptInstance->InvokeOnTriggerStay((unsigned int)ent);
            }
        }

    }

    void ScriptSystem::OnFixedUpdate(float dt)
    {
        mTimeInstance->InvokeOnFixedUpdate(dt);

        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            auto& scriptComponent = mRegistry->get<Script>(id);

            //if disabled should not update
            if (!scriptComponent.componentEnabled)
            {
                continue;
            }

            scriptRef->InvokeOnFixedUpdate(dt);
            UpdateScriptComponent(id);

        }

        //// Loop through all entity instances
        //for (const auto& [id, scriptRef] : mEntityInstances)
        //{
        //    auto& scriptComponent = mRegistry->get<Script>(id);

        //    //if disabled should not update
        //    if (!scriptComponent.componentEnabled)
        //    {
        //        continue;
        //    }

        //}


    }

    void ScriptSystem::OnLateUpdate(float dt)
    {
        mTimeInstance->InvokeOnLateUpdate(dt);

        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            auto& scriptComponent = mRegistry->get<Script>(id);

            //if disabled should not update
            if (!scriptComponent.componentEnabled)
            {
                continue;
            }

            scriptRef->InvokeOnLateUpdate(dt);
            UpdateScriptComponent(id);
        }

        // Loop through all entity instances
        //for (const auto& [id, scriptRef] : mEntityInstances)
        //{
        //    auto& scriptComponent = mRegistry->get<Script>(id);

        //    //if disabled should not update
        //    if (!scriptComponent.componentEnabled)
        //    {
        //        continue;
        //    }

        //}


    }

    /// <summary>
    /// the only use for this is if a new entity is created in the editor
    /// and a script is assigned after having a script component
    /// this is to update the script component and create a script instance of the entity
    /// entities created in runtime/play mode should be done as a prefab and should
    /// already have a script component when deserialized so it should be loaded in EntityAdded
    /// Only time this might break is if they add an entity and script in runtime in the editor..
    /// </summary>
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

                    mEntityInstances[*entity] = scriptObj;

                    UpdateScriptComponent(*entity);
                    entityAdded.erase(entity);
                    break;
                    // entity = entityAdded.begin();
                }
            }
        }

        if (Core::GetInstance()->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE)
        {
            

            //for (auto entity : entityToInit)
            //{
            //    auto& scriptComponent = mRegistry->get<Script>(entity);

            //    std::shared_ptr<ScriptObject> instance = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
            //    mEntityInstances[entity] = instance;

            //    // in the script component
            //    UpdateScriptVariables(entity);

            //    // idk incase it isnt populated the first time
            //    UpdateScriptComponent(entity);

            //    mEntityInstances[entity]->InvokeOnConstruct((unsigned int)entity);
            //}

            for(auto entity: entityToInit)
            {
                if (mEntityInstances.count(entity) == 0)
                {
                            auto& scriptComponent = mRegistry->get<Script>(entity);

                            std::shared_ptr<ScriptObject> instance = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
                            mEntityInstances[entity] = instance;

                            // in the script component
                            UpdateScriptVariables(entity);

                            // idk incase it isnt populated the first time
                            UpdateScriptComponent(entity);

                            mEntityInstances[entity]->InvokeOnConstruct((unsigned int)entity);

                }
                mEntityInstances[entity]->InvokeOnAwake();
            }

            for (auto entity : entityToInit)
            {
                mEntityInstances[entity]->InvokeOnCreate();
            }

            entityToInit.clear();
        }


    }

    void ScriptSystem::OnEnd()
    {
        ClearManagedHandles();

        for (auto& it : mEntityInstances)
        {
            if (it.second->mHandle)
            {
                mono_gchandle_free(it.second->mHandle);
                it.second->mHandle = 0;
            }
        }

        mRegistry->on_construct<InactiveEntity>().disconnect<&ScriptSystem::OnDisabled>(this);
        mRegistry->on_destroy<InactiveEntity>().disconnect<&ScriptSystem::OnEnabled>(this);

        mCollisionQueue.clear();
        //mEntityCollisionMap.clear();
        //mEntitiesDisabled.clear();
        mCollideMap.clear();
        mTriggerMap.clear();
        mEntityInstances.clear();
        entityAdded.clear();

        // Stop all active coroutines when changing scene
        // incase someone attaches a coroutine to a slicebehaviour that isn't an entity
        // and thus won't be destroyed when entityDestroyed.
        if (mCoroutineManager)
        {
            MonoMethod* StopAllCoroutines = mCoroutineManager->GetMethod("OnEnd", 0);
            if (StopAllCoroutines)
                mCoroutineManager->InvokeMethod(mCoroutineInstance->mMonoInstance, StopAllCoroutines);
        }
    }

    /// <summary>
    /// used for updating the variables in teh script instance
    /// based on the variabels in teh script map
    /// mostly used when an entity is loaded in, use the variables from
    /// the serialize'd script map to update the variables in the script instance
    /// </summary>
    /// <param name="entity">Entity to update</param>
    void ScriptSystem::UpdateScriptVariables(Entity entity)
    {
        auto& scriptComponent = mRegistry->get<Script>(entity);

        auto& scriptRef = mEntityInstances[entity];
        const auto& fields = scriptRef->GetScriptClass()->mFields;
        for (const auto& it : fields)
        {
            if (it.second.mElementClass == nullptr)
            {
                auto entry = scriptComponent.scriptableFieldMap.find(it.first);

                if (entry != scriptComponent.scriptableFieldMap.end())
                {
                    rttr::variant& v = entry->second;

                    // if this isnt a valid variant
                    if (!v.is_valid())
                    {
                        SLICE_LOG_ERROR("Invalid/null variant for scriptable field map");
                        continue;
                    }

                    if (it.second.mType == ScriptFieldType::String)
                    {
                        if (v.is_type<std::string>())
                        {
                            std::string str = scriptComponent.scriptableFieldMap[it.first].get_value<std::string>();
                            scriptRef->SetFieldValue<std::string>(it.second.mName, str);
                        }
                        else
                        {
                            SLICE_LOG_ERROR("Mismach type.");

                        }
                    }
                    else if (it.second.mType == ScriptFieldType::GameObject)
                    {
                        rttr::variant& variantVal = scriptComponent.scriptableFieldMap[it.first];
                        if (variantVal.is_type<GameObject>())
                        {
                            GameObject go = variantVal.get_value<GameObject>();
                            scriptRef->SetFieldValue(it.second.mName.c_str(), go);
                        }

                    }
                    else if (it.second.mType == ScriptFieldType::Prefab)
                    {
                        rttr::variant& variantVal = scriptComponent.scriptableFieldMap[it.first];
                        if (variantVal.is_type<PrefabVar>())
                        {
                            PrefabVar var = variantVal.get_value<PrefabVar>();
                            scriptRef->SetFieldValue(it.second.mName.c_str(), var);
                        }
                    }
                    else
                    {
                        scriptRef->SetFieldValue(it.second.mName.c_str(), scriptComponent.scriptableFieldMap[it.first]);
                    }
                }
            }
            else
            {
                auto entry = scriptComponent.scriptableFieldMap.find(it.first);

                if (entry != scriptComponent.scriptableFieldMap.end())
                {
                    rttr::variant& v = entry->second;

                    if (!v.is_valid())
                    {
                        SLICE_LOG_ERROR("Invalid variant for an array field");
                        continue;
                    }
                    if (!v.get_type().is_sequential_container())
                    {
                        continue;
                    }

                    // if its an array
                    if (it.second.mContainerType == ScriptFieldType::Array)
                    {
                        scriptRef->SetFieldValue(it.second.mName.c_str(), v);
                    }
                    // if its a list
                    else if (it.second.mContainerType == ScriptFieldType::List)
                    {
                        MonoObject* listObject = scriptRef->GetListObject(it.second.mName);
                        if (listObject == nullptr || it.second.mListClear == nullptr)
                        {
                            SLICE_LOG_ERROR("List " + it.first + " is null or Clear() isn't defined");
                            continue;
                        }

                        // clear the list first before adding from the serialized vector
                        scriptRef->mScriptClass->InvokeMethod(listObject, it.second.mListClear, nullptr);

                        // store the vector that is a variant into a sequential view to iterate
                        rttr::variant_sequential_view view = v.create_sequential_view();

                        for (size_t i = 0; i < view.get_size(); ++i)
                        {
                            rttr::variant item = view.get_value(i);
                            // idk why but any lists/arrays have to be unwrapped if not its garbage values
                            if (item.get_type().is_wrapper())
                            {
                                item = item.extract_wrapped_value();
                            }

                            // TODO: add any extra variables if needed but I dont think we need more than this
                            // maybe find out how to do GameObject/Prefab as variables which are technically just string under the hood
                            switch (it.second.mType)
                            {
                            case ScriptFieldType::Float:
                                scriptRef->AddListFieldValue<float>(it.second.mName, item.get_value<float>());
                                break;
                            case ScriptFieldType::Int:
                                scriptRef->AddListFieldValue<int>(it.second.mName, item.get_value<int>());
                                break;
                            case ScriptFieldType::String:
                                scriptRef->AddListFieldValue<std::string>(it.second.mName, item.get_value<std::string>());
                                break;
                            case ScriptFieldType::Vector3:
                                scriptRef->AddListFieldValue<glm::vec3>(it.second.mName, item.get_value<glm::vec3>());
                                break;
                            case ScriptFieldType::GameObject:
                                scriptRef->AddListFieldValue<GameObject>(it.second.mName, item.get_value<GameObject>());
                                break;
                            case ScriptFieldType::Prefab:
                                //scriptRef->AddListFieldValue<PrefabVar>(it.second.mName, item.get_value<PrefabVar>());
                                break;
                            }
                        }
                    }

                }
            }
        }
    }

    /// <summary>
    /// Used for updating the script map based on the variables in the script instance
    /// Mostly used in editor so when ppl modify the variables
    /// itll reflect in the component so it can be serialized properly
    /// </summary>
    /// <param name="entity"></param>
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
                // If mElementClass is not a nullptr
                // then its an array or list
                if (it.second.mElementClass != nullptr)
                {
                    if (it.second.mContainerType == ScriptFieldType::Array)
                    {
                        if (it.second.mType == ScriptFieldType::Float)
                        {
                            std::vector<float> var = scriptRef->GetArrayFieldValue<float>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::Bool)
                        {
                            std::vector<bool> var = scriptRef->GetArrayFieldValue<bool>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::String)
                        {
                            std::vector<std::string> var = scriptRef->GetArrayFieldValue<std::string>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::Int)
                        {
                            std::vector<int> var = scriptRef->GetArrayFieldValue<int>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::Vector3)
                        {
                            std::vector<glm::vec3> var = scriptRef->GetArrayFieldValue<glm::vec3>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        //else if (it.second.mType == ScriptFieldType::GameObject)
                        //{
                        //    //GameObject var = scriptRef->GetArrayFieldValue<GameObject>(it.second.mName);
                        //    //scriptComponent.scriptableFieldMap[it.first] = var;

                        //    // test

                        //}
                    }
                    else if (it.second.mContainerType == ScriptFieldType::List)
                    {
                        if (it.second.mType == ScriptFieldType::Float)
                        {
                            std::vector<float> var = scriptRef->GetListFieldValue<float>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::Bool)
                        {
                            std::vector<bool> var = scriptRef->GetListFieldValue<bool>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::String)
                        {
                            std::vector<std::string> var = scriptRef->GetListFieldValue<std::string>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::Int)
                        {
                            std::vector<int> var = scriptRef->GetListFieldValue<int>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::Vector3)
                        {
                            std::vector<glm::vec3> var = scriptRef->GetListFieldValue<glm::vec3>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        else if (it.second.mType == ScriptFieldType::GameObject)
                        {
                            std::vector<GameObject> var = scriptRef->GetListFieldValue<GameObject>(it.second.mName);
                            scriptComponent.scriptableFieldMap[it.first] = var;
                        }
                        //else if (it.second.mType == ScriptFieldType::Prefab)
                        //{
                        //    std::vector<PrefabVar> var = scriptRef->GetListFieldValue<PrefabVar>(it.second.mName);
                        //    scriptComponent.scriptableFieldMap[it.first] = var;
                        //}
                    }
                }
                else if (it.second.mType == ScriptFieldType::Float)
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
                else if (it.second.mType == ScriptFieldType::Vector3)
                {
                    glm::vec3 var = scriptRef->GetFieldValue<glm::vec3>(it.second.mName);
                    scriptComponent.scriptableFieldMap[it.first] = var;
                }
                else if (it.second.mType == ScriptFieldType::GameObject)
                {
                    GameObject var = scriptRef->GetFieldValue<GameObject>(it.second.mName);
                    scriptComponent.scriptableFieldMap[it.first] = var;

                    // test
                   
                }
                else if (it.second.mType == ScriptFieldType::Prefab)
                {
                    // this shit broken
                    //rttr::variant prefabVar = scriptRef->GetFieldValue(it.second.mName);
                    //scriptComponent.scriptableFieldMap[it.first] = prefabVar;

                    PrefabVar var = scriptRef->GetFieldValue<PrefabVar>(it.second.mName);
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
            ////std::cout << "Loading" << scriptComponent.scriptName << "for entity " << (unsigned int)entity << std::endl;
            ////std::cout << "curr state " << Core::GetInstance()->GetSceneSystem()->mCurrentState << std::endl;
            ////std::cout << "next state " << Core::GetInstance()->GetSceneSystem()->mNextState << std::endl;

            if(Core::GetInstance()->GetSceneSystem()->mCurrentState != SceneState::PLAY_SCENE && Core::GetInstance()->GetSceneSystem()->mNextState == SceneState::PLAY_SCENE)
            {
                //static bool tempFlagToTestScriptListShit = false;
                std::shared_ptr<ScriptObject> instance = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
                mEntityInstances[entity] = instance;

                // in the script component
                UpdateScriptVariables(entity);

                // idk incase it isnt populated the first time
                UpdateScriptComponent(entity);
                entityConstructed.insert(entity);
                // construct first but awake and create after all entities has been resolved
                mEntityInstances[entity]->InvokeOnConstruct((unsigned int)entity);
                mEntityInstances[entity]->InvokeOnAwake();
                mEntityInstances[entity]->InvokeOnCreate();

            }
            else if (Core::GetInstance()->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE)
             {
                if (isChangingScene)
                    entityToInit.insert(entity);

                std::shared_ptr<ScriptObject> instance = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
                mEntityInstances[entity] = instance;

                // in the script component
                UpdateScriptVariables(entity);

                // idk incase it isnt populated the first time
                UpdateScriptComponent(entity);
                mEntityInstances[entity]->InvokeOnConstruct((unsigned int)entity);
                if (!isChangingScene)
                {
                    mEntityInstances[entity]->InvokeOnAwake();
                    mEntityInstances[entity]->InvokeOnCreate();
                }
            //mEntityInstances[entity]->InvokeOnConstruct((unsigned int)entity);


            //mEntityInstances[entity]->InvokeOnAwake();
            //mEntityInstances[entity]->InvokeOnCreate();

            }
            else // in normal editor mode
            {
                //static bool tempFlagToTestScriptListShit = false;
                std::shared_ptr<ScriptObject> instance = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
                mEntityInstances[entity] = instance;

                // in the script component
                UpdateScriptVariables(entity);

                // idk incase it isnt populated the first time
                UpdateScriptComponent(entity);

            }
        }
        else
        {
            //std::cout << "no script component name " << "for entity " << (unsigned int)entity << std::endl;

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
        mCoroutineInstance->InvokeOnEntityDestroy(static_cast<unsigned int>(entity));
        


        {
            std::lock_guard<std::mutex> lock(mQueueLock);
            mCollisionQueue.erase(
                std::remove_if(mCollisionQueue.begin(), mCollisionQueue.end(),
                    [entity](const QueuedCollisionEvent& ev) {
                        // Remove if the script-owning entity OR the 'other' entity is gone
                        return ev.entity == entity || ev.other == entity;
                    }),
                mCollisionQueue.end()
            );
        }

        for (auto& it : mEntityInstances)
        {
            if (it.first == entity)
            {
                it.second->InvokeOnEntityDestroy((unsigned int)entity);

    //            mono_gchandle_free(it.second->mHandle);
				//it.second->mHandle = 0;
                it.second->Destroy();

                mEntityInstances.erase(it.first);
                break;
            }
        }

        auto it = mManagedGameObjectHandles.find(entity);
        if (it != mManagedGameObjectHandles.end())
        {
            mono_gchandle_free(it->second);
            mManagedGameObjectHandles.erase(it);
        }

        for (auto it2 = entityAdded.begin(); it2 != entityAdded.end(); ++it2)
        {
            if (*it2 == entity)
            {
                entityAdded.erase(it2);
                break;
            }
        }
        
        for (auto it2 = entityToInit.begin(); it2 != entityToInit.end(); ++it2)
        {
            if (*it2 == entity)
            {
                entityToInit.erase(it2);
                break;
            }
        }

        //mEntitiesDisabled.erase(entity);
        //mEntityCollisionMap.erase(entity); 

        mCollideMap.erase(entity);
        mTriggerMap.erase(entity);

        //for (auto& [otherEntity, collisionSet] : mEntityCollisionMap)
        //{
        //    collisionSet.erase(entity);
        //}
    }

    void ScriptSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
    {
        // idk if its a sequence issue or thread issue or what but updating here doesnt work
        // so I update after every onUpdate call for any thing script related
        // Editor calls it when anything is modified in the inspector as well
        //UpdateScriptComponent(entity);
    }

    void ScriptSystem::OnEnabled(entt::registry& reg, entt::entity entity)
    {
        if (Core::GetInstance()->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE)
        {
            for (auto& [entt, instance] : mEntityInstances)
            {
                if (entt == entity)
                {
                    //mEntitiesDisabled.emplace(entt);
                    // invoke onEnabled
                    instance->InvokeOnEnabled();
                }
            }
        }
    }

    void ScriptSystem::OnDisabled(entt::registry& reg, entt::entity entity)
    {
        if (Core::GetInstance()->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE)
        {
            for (auto& [entt, instance] : mEntityInstances)
            {
                if (entt == entity)
                {
                    // invoke onDisabled
                    instance->InvokeOnDisabled();
                }
            }
        }
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
            uint32_t visibility = flags & MONO_TYPE_ATTR_VISIBILITY_MASK;

            // Skip all nested types (nested types have visibility values 0x02–0x06)
            if (visibility >= MONO_TYPE_ATTR_NESTED_PUBLIC && visibility <= MONO_TYPE_ATTR_NESTED_FAM_OR_ASSEM)
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
                        if (mono_field_get_flags(field) & MONO_FIELD_ATTR_PUBLIC)
                        {
                            MonoType* type = mono_field_get_type(field);

                            std::string fieldTypeStr = mono_type_get_name(type);

                            MonoClass* elementClass = nullptr;
                            ScriptFieldType containerType = ScriptFieldType::None;
                            ScriptFieldType fieldType = GetScriptFieldType(type, &elementClass, containerType);
                            //if (fieldType == ScriptFieldType::GameObject && containerType != ScriptFieldType::None)
                            //    continue;
                            if (fieldType == ScriptFieldType::Prefab )
                                continue;
                            /*
                            MonoTypeEnum e = (MonoTypeEnum)mono_type_get_type(type);*/
                           /* if (e == MONO_TYPE_SZARRAY || e == MONO_TYPE_ARRAY)
                            {
                                SLICE_LOG(fieldTypeStr + "is an array!");
                            }

                            if (mono_type_get_array_type(type)&& !(e == MONO_TYPE_SZARRAY || e == MONO_TYPE_ARRAY || e == MONO_TYPE_GENERICINST))
                            {
                                SLICE_LOG_CRITICAL("This type " + fieldTypeStr + " in " + className +  " is considered an array type.");

                                if (!mono_type_is_struct(type))
                                {
                                    SLICE_LOG_VALUES("However, it is not a struct");
                                }
                                else
                                {
                                    SLICE_LOG_VALUES("It is a struct");
                                }
                            }*/
                            rttr::variant var;
                            // Store it in the script's field map
                            script->mFields[fieldName] = { fieldType, containerType, fieldName, field, var, elementClass };

                            // if its a list, then we have to cache some functions to help
                            // with list interacting
                            if (containerType == ScriptFieldType::List)
                            {
                                ScriptField& field2 = script->mFields[fieldName];

                                // store the List class so we can get its methods
                                field2.mCollectionClass = mono_class_from_mono_type(type);

                                field2.mListCtor = mono_class_get_method_from_name(field2.mCollectionClass, ".ctor", 0);

                                MonoProperty* propCount = mono_class_get_property_from_name(field2.mCollectionClass, "Count");
                                if (propCount)
                                    field2.mListGetCount = mono_property_get_get_method(propCount);

                                field2.mListGetItem = mono_class_get_method_from_name(field2.mCollectionClass, "get_Item", 1);
                                field2.mListSetItem = mono_class_get_method_from_name(field2.mCollectionClass, "set_Item", 2);

                                field2.mListAdd = mono_class_get_method_from_name(field2.mCollectionClass, "Add", 1);
                                field2.mListClear = mono_class_get_method_from_name(field2.mCollectionClass, "Clear", 0);
                                field2.mListRemoveAt = mono_class_get_method_from_name(field2.mCollectionClass, "RemoveAt", 1);
                            }
                        }
                    }

                    currentClass = mono_class_get_parent(currentClass);
                    if (currentClass == entityClass)
                        break;
                }
            }
        }
    }

    void ScriptSystem::ReloadEntityScript(Entity entity)
    {
        auto& scriptComponent = mRegistry->get<Script>(entity);

        if (mEntityInstances.count(entity) > 0)
        {
            mono_gchandle_free(mEntityInstances[entity]->mHandle);
            mEntityInstances[entity]->mHandle = 0;
            mEntityInstances.erase(entity);
        }

        if (HasEntityClass(scriptComponent.scriptName))
        {
            std::shared_ptr<ScriptObject> scriptObj = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
            mEntityInstances[entity] = scriptObj;
            UpdateScriptVariables(entity);
            UpdateScriptComponent(entity);
		}
    }

    ScriptFieldType ScriptSystem::GetScriptFieldType(MonoType* type, MonoClass** outElementClass, ScriptFieldType& containerType)
    {
        *outElementClass = nullptr;

        std::string fullTypeName = mono_type_get_name(type);
        


        std::string listPrefix = "System.Collections.Generic.List<";

        // cause full type name is System.Collections.Generic.List<soemthing>
        // rfind returns the starting position of the last occurance which is basically jus checking
        // if the front part is the same as the listPrefix and that it starts from the front
        if (fullTypeName.rfind(listPrefix, 0) == 0 && fullTypeName.back() == '>')
        {
            // if its here means it is a List<T>

            size_t nameStart = listPrefix.length();
            // -1 cause > at the back of the full name
            size_t nameLength = fullTypeName.length() - nameStart - 1;
            std::string elementType = fullTypeName.substr(nameStart, nameLength);
            if (sFieldTypeMap.count(elementType))
            {


                std::string nameSpace;
                std::string className;
                size_t dotPos = elementType.find_last_of('.');
                if (dotPos == std::string::npos)
                {
                    // its some random class thats not in slice engine namespace
                    nameSpace = "";
                    className = elementType;
                }
                else
                {
                    // split into SliceEngine and Vector3
                    nameSpace = elementType.substr(0, dotPos);
                    className = elementType.substr(dotPos + 1);
                }
                MonoImage* image = nullptr;
                // check if its a System variable or a SliceEngine variable
                // i.e float or vec3 or smth
                if (nameSpace == "System")
                {
                    // get core lib for system varaibles
                    image = mono_get_corlib();
                }
                else if (nameSpace == "SliceEngine")
                {
                    image = mCoreAssemblyImage;
                }
                else
                {
                    SLICE_LOG_ERROR("Unsupported variable: " + elementType);
                    return ScriptFieldType::None;

                }

                containerType = ScriptFieldType::List;
                *outElementClass = mono_class_from_name(image, nameSpace.c_str(), className.c_str());
                ScriptFieldType baseType = sFieldTypeMap.at(elementType);
                return baseType;
            }
        }


        MonoArrayType* arrayType = mono_type_get_array_type(type);
        mono_bool isStruct = mono_type_is_struct(type);
        MonoTypeEnum enumType = (MonoTypeEnum)mono_type_get_type(type);
        if ((enumType == MONO_TYPE_ARRAY || enumType == MONO_TYPE_SZARRAY) && !isStruct)
        {
            MonoClass* elementClass = arrayType->eklass;
            SLICE_LOG_DEBUG(mono_class_get_name(elementClass));
            *outElementClass = elementClass;

            MonoType* elementType = mono_class_get_type(elementClass);
            std::string elementTypeName = mono_type_get_name(elementType);

            if (sFieldTypeMap.count(elementTypeName))
            {
                auto iter = sFieldTypeMap.find(elementTypeName);

                containerType = ScriptFieldType::Array;
                return iter->second;
            }
        }
        else
        {
            std::string name = mono_type_get_name(type);
            // If the name exist in our field type map
            if (sFieldTypeMap.count(name) != 0)
            {
                auto iter = sFieldTypeMap.find(name);
                return iter->second;
            }
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

        //Collision System
        eventManager->Subscribe<OnCollisionEnterEvent, &ScriptSystem::OnCollideEnter>(this);

        eventManager->Subscribe<OnCollisionStayEvent, &ScriptSystem::OnCollideStay>(this);

        eventManager->Subscribe<OnCollisionExitEvent, &ScriptSystem::OnCollideExit>(this);

        eventManager->Subscribe<OnTriggerEnterEvent, &ScriptSystem::OnTriggerEnter>(this);

        eventManager->Subscribe<OnTriggerStayEvent, &ScriptSystem::OnTriggerStay>(this);

        eventManager->Subscribe<OnTriggerExitEvent, &ScriptSystem::OnTriggerExit>(this);

        //UI System
        eventManager->Subscribe<OnButtonClickEvent, &ScriptSystem::OnButtonClick>(this);
        eventManager->Subscribe<OnButtonHoverEvent, &ScriptSystem::OnButtonHover>(this);
        eventManager->Subscribe<OnButtonExitHoverEvent, &ScriptSystem::OnButtonExitHover>(this);
        eventManager->Subscribe<OnButtonReleaseEvent, &ScriptSystem::OnButtonRelease>(this);
        eventManager->Subscribe<OnSliderValueEvent, &ScriptSystem::OnSliderValue>(this);
        eventManager->Subscribe<OnSpriteAnimStopEvent, &ScriptSystem::OnSpriteAnimStop>(this);
        eventManager->Subscribe<OnSpriteAnimLoopEvent, &ScriptSystem::OnSpriteAnimLoop>(this);

        eventManager->Subscribe< AnimationEvent, &ScriptSystem::OnAnimationEvent>(this);
    }

    void ScriptSystem::UnsubscribeToEvents()
    {
        auto* eventManager = EventManager::GetInstance();

        eventManager->Unsubscribe<OnCollisionEnterEvent, &ScriptSystem::OnCollideEnter>(this);

        eventManager->Unsubscribe<OnCollisionStayEvent, &ScriptSystem::OnCollideStay>(this);

        eventManager->Unsubscribe<OnCollisionExitEvent, &ScriptSystem::OnCollideExit>(this);

        eventManager->Unsubscribe<OnTriggerEnterEvent, &ScriptSystem::OnTriggerEnter>(this);

        eventManager->Unsubscribe<OnTriggerStayEvent, &ScriptSystem::OnTriggerStay>(this);

        eventManager->Unsubscribe<OnTriggerExitEvent, &ScriptSystem::OnTriggerExit>(this);

    }

    void ScriptSystem::RemapGameObjectVariables(const std::unordered_map<uint32_t, uint32_t>& sceneGraph)
    {
        for (auto& [entity, scriptInstance] : mEntityInstances)
        {
            FixGOVariables(sceneGraph, entity, scriptInstance);
        }
    }

    void ScriptSystem::RemapPrefabVariables(const std::unordered_map<uint32_t, uint32_t>& sceneGraph, Entity entity)
    {
        if (mEntityInstances.find(entity) == mEntityInstances.end())
        {
            return;
        }

        FixGOVariables(sceneGraph, entity, mEntityInstances[entity]);
    }

    void ScriptSystem::FixGOVariables(const std::unordered_map<uint32_t, uint32_t>& sceneGraph, Entity entity, std::shared_ptr<ScriptObject>& scriptInstance)
    {
        auto& scriptComponent = mRegistry->get<Script>(entity);

        for (auto& [fieldName, variantVal] : scriptComponent.scriptableFieldMap)
        {
            if (variantVal.is_type<GameObject>())
            {
                GameObject go = variantVal.get_value<GameObject>();
                uint32_t oldID = (uint32_t)go.GetEntity();

                if (sceneGraph.contains(oldID))
                {
                    uint32_t newID = sceneGraph.at(oldID);
                    GameObject newGO = GameObject(RegistryInstance, (Entity)newID);
                    scriptInstance->SetFieldValue<GameObject>(fieldName, newGO);
                }
            }
            else if (variantVal.is_type <std::vector<GameObject>>())
            {
                // NOTE for gideon
                // i realise, i should probblay be using set index instead because there is already a list existing
                // but with the old IDs
                // but i'm just going to clear and readd again
                // not as efficient but time is of the essence! We must ride at noon.

                std::vector<GameObject> oldGOs = variantVal.get_value<std::vector<GameObject>>();
                MonoObject* listObject = scriptInstance->GetListObject(fieldName);
                if (listObject == nullptr)
                {
                    SLICE_LOG_ERROR("List " + fieldName + " is null or Clear() isn't defined");
                    continue;
                }

                // clear the list first before adding from the serialized vector
                scriptInstance->mScriptClass->InvokeMethod(listObject, scriptInstance->mScriptClass->mFields[fieldName].mListClear, nullptr);

                // clear the list first before adding from the serialized vector
                for (GameObject oldGO : oldGOs)
                {
                    uint32_t oldID = (uint32_t)oldGO.GetEntity();

                    if (sceneGraph.contains(oldID))
                    {
                        uint32_t newID = sceneGraph.at(oldID);
                        GameObject newGO = GameObject(RegistryInstance, (Entity)newID);
                        scriptInstance->AddListFieldValue<GameObject>(fieldName, newGO);
                        //newGOs.push_back(newGO);
                    }
                }
            }
        }

        UpdateScriptComponent(entity);

    }

    void ScriptSystem::QueueCollision(ScriptCollisionType type, Entity entity, Entity otherEntity)
    {
        std::lock_guard<std::mutex> lock(mQueueLock);
        mCollisionQueue.push_back({ type, entity, otherEntity });
    }

    void ScriptSystem::ProcessCollisionQueue()
    {
        std::vector<QueuedCollisionEvent> tempQueue;
        {
            std::lock_guard<std::mutex> lock(mQueueLock);
            if (mCollisionQueue.empty()) return;
            tempQueue.swap(mCollisionQueue);
        }
        //std::cout << tempQueue.size() << std::endl;
        for (const auto& event : tempQueue)
        {
            // make sure entity is still alive
            if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            {
                continue;
            }

            //if the script component is disabled ignore and collision invoke in the script
            auto& scriptComponent = mRegistry->get<Script>(event.entity);
            if (!scriptComponent.componentEnabled)
            {
                continue;
            }

            auto scriptInstance = mEntityInstances[event.entity];

            if (!scriptInstance || scriptInstance->mHandle == 0) continue; // Check for freed handle

            switch (event.type)
            {
            case ScriptCollisionType::CollideEnter:
            {
               // mEntityCollisionMap[event.entity].insert(event.other);
                mCollideMap[event.entity].insert(event.other);
                scriptInstance->InvokeOnCollideEnter((unsigned int)event.other);
            }
                break;
            case ScriptCollisionType::CollideStay:
            {
                //if (mEntitiesDisabled.contains(event.entity))
                //{
                //    // if it was, check if the entity currently colliding with
                //    // had already been collided with before
                //    if (mEntityCollisionMap[event.entity].contains(event.other))
                //    {
                //        // if it has then we want to trigger on enter instead of on stay
                //        // then erase that entity
                //        mEntityCollisionMap[event.entity].erase(event.other);
                //        scriptInstance->InvokeOnCollideEnter((unsigned int)event.other);
                //    }

                //    // if no more entities that it has collided with previously exist
                //    // then erase it from the recently disabled as it has cleared all existing collisions
                //    if (mEntityCollisionMap[event.entity].empty())
                //    {
                //        // mEntityCollisionMap.erase(event.entity);
                //        mEntitiesDisabled.erase(event.entity);
                //    }
                //}
                //else
                //{
                //    scriptInstance->InvokeOnCollideStay((unsigned int)event.other);
                //}
            }
                break;
            case ScriptCollisionType::CollideExit:
            {
                //mEntityCollisionMap[event.entity].erase(event.other);
                mCollideMap[event.entity].erase(event.other);
                scriptInstance->InvokeOnCollideExit((unsigned int)event.other);
            }
                break;
            case ScriptCollisionType::TriggerEnter:
            {
                //mEntityCollisionMap[event.entity].insert(event.other);
                mTriggerMap[event.entity].insert(event.other);
                scriptInstance->InvokeOnTriggerEnter((unsigned int)event.other);
            }
                break;
            case ScriptCollisionType::TriggerStay:
                // check if it was recently re-enabled
                //if (mEntitiesDisabled.contains(event.entity))
                //{
                //    // if it was, check if the entity currently colliding with
                //    // had already been collided with before
                //    if (mEntityCollisionMap[event.entity].contains(event.other))
                //    {
                //        // if it has then we want to trigger on enter instead of on stay
                //        // then erase that entity
               // //        mEntityCollisionMap[event.entity].erase(event.other);
                //        scriptInstance->InvokeOnTriggerEnter((unsigned int)event.other);
                //    }

                //    // if no more entities that it has collided with previously exist
                //    // then erase it from the recently disabled as it has cleared all existing collisions
                //    if (mEntityCollisionMap[event.entity].empty())
                //    {
                //       // mEntityCollisionMap.erase(event.entity);
                //        mEntitiesDisabled.erase(event.entity);
                //    }

                //}
                //else
                //{
                //   scriptInstance->InvokeOnTriggerStay((unsigned int)event.other);
                //}
                break;
            case ScriptCollisionType::TriggerExit:
            {
               // mEntityCollisionMap[event.entity].erase(event.other);
                mTriggerMap[event.entity].erase(event.other);
                scriptInstance->InvokeOnTriggerExit((unsigned int)event.other);
            }
                break;
            }
        }
    }

    void ScriptSystem::OnCollideEnter(const OnCollisionEnterEvent& event)
    {

        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        QueueCollision(ScriptCollisionType::CollideEnter, event.entity, event.other);
        //auto scriptInstance = mEntityInstances[event.entity];
        //if (scriptInstance)
        //{

        //    //    //std::cout << "On collide being called for " << (uint32_t)event.other << std::endl;
        //    scriptInstance->InvokeOnCollideEnter((unsigned int)event.other);
        //}
    }
    void ScriptSystem::OnCollideStay(const OnCollisionStayEvent& event)
    {

        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;
        QueueCollision(ScriptCollisionType::CollideStay, event.entity, event.other);

        //auto scriptInstance = mEntityInstances[event.entity];
        //if (scriptInstance)
        //{
        //    scriptInstance->InvokeOnCollideStay((unsigned int)event.other);
        //}
    }
    void ScriptSystem::OnCollideExit(const OnCollisionExitEvent& event)
    {

        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;
        QueueCollision(ScriptCollisionType::CollideExit, event.entity, event.other);

        //auto scriptInstance = mEntityInstances[event.entity];
        //if (scriptInstance)
        //{
        //    scriptInstance->InvokeOnCollideExit((unsigned int)event.other);
        //}
    }
    void ScriptSystem::OnTriggerEnter(const OnTriggerEnterEvent& event)
    {

        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        QueueCollision(ScriptCollisionType::TriggerEnter, event.entity, event.other);

        //auto scriptInstance = mEntityInstances[event.entity];
        //if (scriptInstance)
        //{
        //    scriptInstance->InvokeOnTriggerEnter((unsigned int)event.other);
        //}

    }
    void ScriptSystem::OnTriggerStay(const OnTriggerStayEvent& event)
    {

        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;
        QueueCollision(ScriptCollisionType::TriggerStay, event.entity, event.other);

        //auto scriptInstance = mEntityInstances[event.entity];
        //if (scriptInstance)
        //{
        //    scriptInstance->InvokeOnTriggerStay((unsigned int)event.other);
        //}
    }
    void ScriptSystem::OnTriggerExit(const OnTriggerExitEvent& event)
    {

        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;
        QueueCollision(ScriptCollisionType::TriggerExit, event.entity, event.other);

        //auto scriptInstance = mEntityInstances[event.entity];
        //if (scriptInstance)
        //{
        //    scriptInstance->InvokeOnTriggerExit((unsigned int)event.other);
        //}
    }

    //button funcs
    void ScriptSystem::OnButtonClick(const OnButtonClickEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeButtonOnClick();
        }
    }
    void ScriptSystem::OnButtonHover(const OnButtonHoverEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeButtonOnHover();
        }
    }

    void ScriptSystem::OnButtonExitHover(const OnButtonExitHoverEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeButtonOnExitHover();
        }
    }

    void ScriptSystem::OnButtonRelease(const OnButtonReleaseEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeButtonOnRelease();
        }
    }

    void ScriptSystem::OnSliderValue(const OnSliderValueEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeOnSliderValue(event.value);
        }
    }

    void ScriptSystem::OnSpriteAnimStop(const OnSpriteAnimStopEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeSAnimStop();
        }
    }

    void ScriptSystem::OnSpriteAnimLoop(const OnSpriteAnimLoopEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        if (scriptInstance)
        {
            scriptInstance->InvokeSAnimLoop();
        }
    }


    void ScriptSystem::OnAnimationEvent(const AnimationEvent& event)
    {
        if (mEntityInstances.find(event.entity) == mEntityInstances.end())
            return;

        if (Core::GetInstance()->GetSceneSystem()->mCurrentState != SceneState::PLAY_SCENE)
            return;

        auto scriptInstance = mEntityInstances[event.entity];
        auto scriptClass = scriptInstance->GetScriptClass();

        MonoString* varStr = mono_string_new(mono_domain_get(), event.scriptName.c_str());
        void* param = varStr;
        // TODO: Look into whether we want to allow multiple variables or just a string instead
        // if we do then 1 string for func name, 1 string for the variable
        MonoMethod* eventMethod = scriptClass->GetMethod(event.funcName, 1);
        if (!eventMethod)
        {
            //SLICE_LOG_ERROR("Animation event: Function '{}' not found in script '{}'", event.funcName, scriptClass->mClassName);
            std::string err = "Animation event: Function {";
            err += event.funcName;
            err += "}' not found in script '{";
            err += scriptClass->mClassName;
            err += "}'";

            SLICE_LOG_ERROR(err);
            return;
        }

        // if its here means we can invoke it
        scriptClass->InvokeMethod(scriptInstance->GetInstance(), eventMethod, &param);

        // for now im just going to invoke blank functions to make sure it works
        // look to adding support for either string or x number of variables after this is working.
    }

    MonoObject* ScriptSystem::GetOrCreateManagedObject(Entity entity)
    {
        if (entity == entt::null) return nullptr;
     
        if (mono_domain_get() != mAppDomain)
        {
            mono_thread_attach(mRootDomain);
            mono_domain_set(mAppDomain, false);
        }

        if (mManagedGameObjectHandles.find(entity) != mManagedGameObjectHandles.end())
        {
            MonoObject* obj = mono_gchandle_get_target(mManagedGameObjectHandles[entity]);
            if (obj) return obj;

            mono_gchandle_free(mManagedGameObjectHandles[entity]);
            mManagedGameObjectHandles.erase(entity);
        }

        MonoClass* gameObjectClass = mono_class_from_name(mCoreAssemblyImage, "SliceEngine", "GameObject");
        MonoObject* managedInstance = mono_object_new(mAppDomain, gameObjectClass);

        MonoMethod* ctor = mono_class_get_method_from_name(gameObjectClass, ".ctor", 1);
        uint32_t entityID = (uint32_t)entity;
        void* args[1] = { &entityID };
        mono_runtime_invoke(ctor, managedInstance, args, nullptr);

        uint32_t handle = mono_gchandle_new(managedInstance, false);
        mManagedGameObjectHandles[entity] = handle;

        return managedInstance;
    }
    void ScriptSystem::ClearManagedHandles()
    {
        for (auto& [entity, handle] : mManagedGameObjectHandles)
        {
            mono_gchandle_free(handle);
        }

        mManagedGameObjectHandles.clear();
    }
}
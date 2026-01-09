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

        LoadMonoAssembly("../SliceScript/SliceScript.dll");

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


            scriptRef->InvokeOnConstruct((unsigned int)id);
            scriptRef->InvokeOnCreate();
            UpdateScriptComponent(id);
        }
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
    }

    void ScriptSystem::OnFixedUpdate(float dt)
    {
        mTimeInstance->InvokeOnFixedUpdate(dt);

        // Loop through all entity instances
        for (const auto& [id, scriptRef] : mEntityInstances)
        {
            scriptRef->InvokeOnFixedUpdate(dt);
            UpdateScriptComponent(id);
        }
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
            //static bool tempFlagToTestScriptListShit = false;
            std::shared_ptr<ScriptObject> instance = std::make_shared<ScriptObject>(mEntityClasses[scriptComponent.scriptName], entity);
            mEntityInstances[entity] = instance;


            // Update the variables in script instance with variables 
            // in the script component
            UpdateScriptVariables(entity);

            //if (scriptComponent.scriptName == "SliceEngine.Spawner" && tempFlagToTestScriptListShit == false)
            //{ 
            //    tempFlagToTestScriptListShit = true;
            //// jus testing if add list field value worked
            //    // i need test if serializing it works first
            //    mEntityInstances[entity]->AddListFieldValue("testList", 2.0f);
            //}

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
        mCoroutineInstance->InvokeOnEntityDestroy(static_cast<unsigned int>(entity));

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
        // idk if its a sequence issue or thread issue or what but updating here doesnt work
        // so I update after every onUpdate call for any thing script related
        // Editor calls it when anything is modified in the inspector as well
        //UpdateScriptComponent(entity);
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

                            MonoTypeEnum e = (MonoTypeEnum)mono_type_get_type(type);
                            if (e == MONO_TYPE_SZARRAY || e == MONO_TYPE_ARRAY)
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
                            }
                            rttr::variant var;
                            // Store it in the script's field map
                            script->mFields[fieldName] = { fieldType, containerType, fieldName, field, var, elementClass };

                            // if its a list, then we have to cache some functions to help
                            // with list interacting
                            if (containerType == ScriptFieldType::List)
                            {
                                ScriptField& field = script->mFields[fieldName];

                                // store the List class so we can get its methods
                                field.mCollectionClass = mono_class_from_mono_type(type);

                                field.mListCtor = mono_class_get_method_from_name(field.mCollectionClass, ".ctor", 0);

                                MonoProperty* propCount = mono_class_get_property_from_name(field.mCollectionClass, "Count");
                                if (propCount)
                                    field.mListGetCount = mono_property_get_get_method(propCount);

                                field.mListGetItem = mono_class_get_method_from_name(field.mCollectionClass, "get_Item", 1);
                                field.mListSetItem = mono_class_get_method_from_name(field.mCollectionClass, "set_Item", 2);

                                field.mListAdd = mono_class_get_method_from_name(field.mCollectionClass, "Add", 1);
                                field.mListClear = mono_class_get_method_from_name(field.mCollectionClass, "Clear", 0);
                                field.mListRemoveAt = mono_class_get_method_from_name(field.mCollectionClass, "RemoveAt", 1);
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
        eventManager->Subscribe<OnButtonReleaseEvent, &ScriptSystem::OnButtonRelease>(this);
        eventManager->Subscribe<OnSliderValueEvent, &ScriptSystem::OnSliderValue>(this);

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

            if (!scriptInstance) continue;

            switch (event.type)
            {
            case ScriptCollisionType::CollideEnter:
                scriptInstance->InvokeOnCollideEnter((unsigned int)event.other);
                break;
            case ScriptCollisionType::CollideStay:
                scriptInstance->InvokeOnCollideStay((unsigned int)event.other);
                break;
            case ScriptCollisionType::CollideExit:
                scriptInstance->InvokeOnCollideExit((unsigned int)event.other);
                break;
            case ScriptCollisionType::TriggerEnter:
                scriptInstance->InvokeOnTriggerEnter((unsigned int)event.other);
                break;
            case ScriptCollisionType::TriggerStay:
                scriptInstance->InvokeOnTriggerStay((unsigned int)event.other);
                break;
            case ScriptCollisionType::TriggerExit:
                scriptInstance->InvokeOnTriggerExit((unsigned int)event.other);
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

        //    //    std::cout << "On collide being called for " << (uint32_t)event.other << std::endl;
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
}
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
#include "ScriptFunctions.h"
#include <filesystem>
#include <fstream>

namespace SliceEngine
{
    ScriptSystem* gScriptSystem = NULL;

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
        {"Carmicah.Vector2", ScriptFieldType::Vector2},
        {"Carmicah.Entity", ScriptFieldType::Entity},
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

    //void ScriptSystem::EntityDestroyed(Entity id)
    //{
    //    // If an entity is destroyed, remove it from mEntityInstances
    //    // can also take this part to call ondestroy if we do that
    //    // mEntityInstances is usually cleared at the end of playing
    //    // but if entity is destroyed in run time then we have to clear it from the map
    //    for (auto& it : mEntityInstances)
    //    {
    //        if (it.first == id)
    //        {
    //            // can call ondestroy here maybe if we do that
    //            //CM_CORE_INFO("Destroying entity {}", id);
    //            mono_gchandle_free(it.second->mHandle);
    //            // erase it from the map
    //            mEntityInstances.erase(it.first);
    //            break;
    //        }
    //    }

    //    for (auto it = entityAdded.begin(); it != entityAdded.end(); ++it)
    //    {
    //        if (*it == id)
    //        {
    //            entityAdded.erase(it);
    //            break;
    //        }
    //    }
    //}

    void ScriptSystem::Init()
    {
        InitMono();

        //ScriptFunctions::RegisterFunctions();

       // LoadEntityClasses();

        //ScriptFunctions::RegisterComponents();

        // PrintAssemblyTypes(mCoreAssembly);
         // retrieve the main Entity class
        //mEntityClass = ScriptClass("Slice", "Entity");

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

        mRootDomain = mono_jit_init("SliceJITRuntime");
        if (mRootDomain == nullptr)
        {
            SLICE_LOG_ERROR("Unable to init mono");
            assert("Failed to init mono jit");
            return;
        }

        //mRootDomain = rootDomain;

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
            scriptRef->InvokeOnConstruct(id);
            scriptRef->InvokeOnCreate();
        }
    }

    void ScriptSystem::OnUpdate(float dt)
    {
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

    }

    void ScriptSystem::OnEnd()
    {
        mEntityInstances.clear();
    }

    void ScriptSystem::UpdateScriptVariables(Entity entity)
    {
        
    }

    void ScriptSystem::UpdateScriptComponent(Entity entity)
    {

    }

    void ScriptSystem::UpdateScriptPrefabComponent(Script& scriptComponent)
    {

    }

    //void ScriptSystem::UpdateAllPrefabScriptComponents()
    //{

    //}

    void ScriptSystem::UpdateExistingPrefabScript(Script& scriptComponent)
    {

    }

    void ScriptSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
    {

    }

    /// <summary>
    /// // If entity has it's scripting component removed 
    /// </summary>
    /// <param name="entity">Entity being removed</param>
    void ScriptSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
    {
        
    }

    void ScriptSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
    {
        
	}

    void ScriptSystem::LoadEntityClasses()
    {
        // clear the map before using it
        mEntityClasses.clear();

        MonoImage* image = mono_assembly_get_image(mCoreAssembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        MonoClass* entityClass = mono_class_from_name(image, "Slice", "Entity");

        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

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
                            //variantVar defaultValue;

                            // Store it in the script's field map
                            //script->mFields[fieldName] = { fieldType, fieldName, field, defaultValue };
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

    std::shared_ptr<ScriptObject> ScriptSystem::GetScriptInstance(unsigned int entityID)
    {
        if (mEntityInstances.count(entityID) == 0)
        {
            return nullptr;
        }

        return mEntityInstances[entityID];
    }
}
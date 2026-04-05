/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AssetManager.h

 author:	  Gideon Nicholas Francis
 co-author:   Nic Lai

 email:       g.francis@digipen.edu

 brief:		  Declares the AssetManager class which handles all asset related operations on the editor side.
			  Creates and manages asset descriptor files, GUIDs, and asset listings.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <fstream>
#include <filesystem>
#include <mutex>
#include <queue>
#include "../thirdparty/filewatch/FileWatcher.h"
#include "json.hpp"
#include "AssetTypes.h"
#include "Core/EditorEvents.h"

namespace SliceEditor
{
	class MetaData;
	
	struct RawFileEvent
	{
		std::filesystem::path filePath;
		filewatch::Event changeType;
	};

	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		void Init();

		void ReloadResource(AssetRecompiledEvent event);

		SliceEngine::GUID ReadGUIDFromDescriptor(std::filesystem::path path);

		/// <summary>
		/// Check if the resource folder meta files are still accurate
		/// meaning, if the original asset path still exist or if
		/// the resource still exist
		/// if not then delete the meta file so that it can be recompiled
		/// </summary>
		/// <param name="path">Resource Folder Path</param>
		//void ScanResourceFolder();
		//::string CreateDescriptorFile(const std::filesystem::path filePath, bool AddToRM = false);
		std::unique_ptr<MetaData> CreateDefaultMeta(const std::filesystem::path filePath);
		void AddDefaultModelsToMap();
		void CreateAssetMaps();
		std::vector<SliceEngine::GUID>* GetMapFromAssetType(std::string assetType);
		std::filesystem::path CreateResource(const std::filesystem::path filePath, MetaData* metaData = nullptr, bool AddToRM = true, bool recompile = false);
		void CompileTextureAsset(std::filesystem::path const& desc_file);
		void CompileFBXAsset(std::filesystem::path const& desc_file);
		void CompileFontAsset(std::filesystem::path const& desc_file);
		void CompileAudioAsset(AudioData* metaData);
		void CompileShaderAsset(ShaderData* metaData);
		void CompileCustomShaderAsset(CustomShaderData* metaData);
		void CompileVertShaderAsset(VertShaderData* metaData);
		void CompileGeomShaderAsset(GeomShaderData* metaData);
		void CompileFragShaderAsset(FragShaderData* metaData);
		void CompileMaterialAsset(MaterialData* metaData);
		void CompileSceneAsset(SceneData* metaData);
		void CompileCSVAsset(CSVData* metaData);
		void CompileNavMeshAsset(NavMeshData* metaData);
		void CompileNavMeshBinAsset(NavMeshBinData* metaData);
		void CompileStateMachineAsset(StateMachineData* metaData);
		void CompileSequencePkgAsset(SequencePkgData* metaData);
		void CompileAnimAsset(SequenceData* metaData);
		void CreatePrefab(SliceEngine::GameObject GO);
		void OnAssetFileSystemEvent(const std::string& path, const filewatch::Event changeType);
		void CleanUpSceneTemp();
		void CreateDefaultAsset(std::filesystem::path& folderPath, AssetType type);
		void CreateAssetManifest();
		void AddDefaultsToManifest(nlohmann::json& manifestJSON);

		void CreateModelGO(SliceEngine::GUID guid, HistoryManager& hist,Entity parent = entt::null, bool isPrefabInspected = false);
		std::filesystem::path GetMetaDataFromFilename(std::string filename);
		

		std::optional<std::string> GetFilenameFromGUID(SliceEngine::GUID guid);
		//std::string TimeToString(std::filesystem::file_time_type ftime);

		std::unordered_map <SliceEngine::GUID, std::string> mGUIDtoFilename; // Maps GUIDs to fileName

		//Main Lookup Table
		std::unordered_map<std::string, SliceEngine::GUID> mFilenameToGUID; // cause asset manager initializes first

		//Sorted Lookup Table: By AssetType (for drop-down lists mostly)
		std::unordered_map<AssetType, std::vector<SliceEngine::GUID>> mAssetTypeToGUIDs
		{
			{ AssetType::Audio, {} },
			{ AssetType::Controller, {} },
			{ AssetType::Material, {} },
			{ AssetType::Model, {} },
			{ AssetType::Texture, {} },
			{ AssetType::Scene, {} },
			{ AssetType::Prefab, {}},
			{ AssetType::CustomShader, {}}
		};

		std::unordered_map <std::string, std::pair<AssetType,std::string>> mSupportedAssetTypes = 
		{
			{".png",  {AssetType::Texture, "Texture"}},
			{".jpg", {AssetType::Texture, "Texture"}},
			{".jpeg", {AssetType::Texture, "Texture"}},
			{".dds", {AssetType::Texture, "Texture"}},
			{".tga", {AssetType::Texture, "Texture"}},
			{".gif", {AssetType::Texture, "Texture"}},
			{".obj", {AssetType::Model, "Model"}},
			{".fbx", {AssetType::Model, "Model"}},
			{".glb", {AssetType::Model, "Model"}},
			{".ttf", {AssetType::Font, "Font"}},
			{".wav", {AssetType::Audio, "Audio"}},
			{".mp3", {AssetType::Audio, "Audio"}},
			{".ogg", {AssetType::Audio, "Audio"}},
			{".scene", {AssetType::Scene, "Scene"}},
			{".temp", {AssetType::Scene, "Scene"}},
			{".shader", {AssetType::Shader, "Shader"}},
			{".cshader", {AssetType::CustomShader, "CustomShader"}},
			{".vert", {AssetType::VertShader, "VertShader"}},
			{".geom", {AssetType::GeomShader, "GeomShader"}},
			{".frag", {AssetType::FragShader, "FragShader"}},
			{".mat", {AssetType::Material, "Material"}},
			{".prefab", {AssetType::Prefab, "Prefab"}},
			{".controller",{AssetType::Controller, "Controller"}},
			{".navmesh",{AssetType::NavMesh, "NavMesh"}},
			{".bin", {AssetType::NavMeshBin, "NavMesh"}},
			{".csv", {AssetType::CSV, "CSV"}},
			{".skl", {AssetType::Skeleton, "Skeleton"}},
			{".animpkg", {AssetType::Animation, "Animation"}},
			{".seqpkg", {AssetType::SequencePackage, "SequencePackage"}},
			{".seq", {AssetType::Sequence, "Sequence"}}
		};

		std::unordered_map <AssetType, std::string> mAssetExtensions =
		{
			{AssetType::Texture, ".dds"},
			{AssetType::Model, ".mdl"},
			{AssetType::Scene, ".scene"},
			{AssetType::Shader, ".shader"},
			{AssetType::CustomShader, ".cshader"},
			{AssetType::VertShader, ".vert"},
			{AssetType::GeomShader, ".geom"},
			{AssetType::FragShader, ".frag"},
			{AssetType::Audio, ".wav"},
			{AssetType::Material, ".mat"},
			{AssetType::Prefab, ".prefab"},
			{AssetType::Skeleton, ".skl"},
			{AssetType::Animation, ".animpkg"},
			{AssetType::SequencePackage, ".seqpkg"},
			{AssetType::Sequence, ".seq"},
			{AssetType::Controller, ".controller" },
			{AssetType::NavMesh, ".navmesh" },
			{AssetType::NavMeshBin, ".bin" },
			{AssetType::CSV, ".csv"},
			{AssetType::Font, ".fnt" }
		};

		std::unordered_map<AssetType, std::string> mDefaultNames =
		{
			{AssetType::Material, "DefaultMaterial"},
			{AssetType::Controller, "DefaultController"},
			{AssetType::CustomShader, "NewCustomShader"}
		};

		
		std::queue<RawFileEvent> mRawFileQueue;
		std::mutex mEventQueueMutex;
		std::filesystem::path mAssetDirectory = std::filesystem::path("../SliceEditor/Assets");
	private:
		
		// TODO: Change this to be configurable
		std::filesystem::path mResourcesDirectory = std::filesystem::path("Resources");

		std::unique_ptr<filewatch::FileWatch<std::string>> mAssetFileWatcher;

		// Gives editor a vector of all asset files by name for displaying in inspector
		//std::unordered_map<AssetType, std::vector<std::string>> mAssets; 

		

	};

}

#endif
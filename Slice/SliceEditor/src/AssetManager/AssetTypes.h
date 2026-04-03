/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AssetTypes.h

 author:	  Gideon Nicholas Francis
 co-author:   Nic Lai

 email:       g.francis@digipen.edu

 brief:		  Defines the Asset types and their meta data structure

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

#include <filesystem>
#include "../../SliceEngine/src/Resource/ResourceManager.h"
#include <Serializer/JSONSerializer.h>
#include "../../SliceEngine/src/Animator/FSMSystem.h"

namespace SliceEditor
{
	enum class AssetType
	{
		Texture,
		Model,
		Skeleton,
		Font,
		Animation,
		SequencePackage,
		Sequence,
		Audio,
		Scene,
		Shader,
		VertShader,
		GeomShader,
		FragShader,
		CustomShader,
		Material,
		Prefab,
		Controller,
		NavMesh,
		NavMeshBin,
		CSV,
		Unsupported
	};
	enum CompressionFormat : std::uint8_t {
		//ima be real idk whats the diff between signed and unsigned, im just yoinking these enums from compressonator
		//same as idk what exactly these are used for, will ask tomas next time
		//ref: https://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/#bc1
		
		//		RGBA_UNCOMPRESSED,
		BC1 = 0,	//RGB + single bit A, color maps, cutout color maps, normal maps
		BC3,	//rgba, color maps with full alpha, packing color and mono maps together
		BC4,	
		BC5,	//2x grayscale, tangent maps
		BC7 	//RGB/RGBA, high quality color maps, color maps with full alpha
	};
	enum UsageType : std::uint8_t {
		Color = 0,
		Tangent_bc5,
		Intensity_bc4
	};

	enum MipMapFilter : std::uint8_t {
		NONE = 0,
		POINT,
		LINEAR,
		TRIANGLE,
		BOX
	};

	enum AudioStream : std::uint8_t
	{
		CREATE_SAMPLE,
		CREATE_STREAM
	};

	

	// type UUIDs 
	namespace ResourceTypeIDs
	{
		constexpr uint64_t TEXTURE = SliceEngine::FNVHash::fnv1a("Texture");
		constexpr uint64_t SHADER = SliceEngine::FNVHash::fnv1a("Shader");
		constexpr uint64_t VERT_SHADER = SliceEngine::FNVHash::fnv1a("VertShader");
		constexpr uint64_t GEOM_SHADER = SliceEngine::FNVHash::fnv1a("GeomShader");
		constexpr uint64_t FRAG_SHADER = SliceEngine::FNVHash::fnv1a("FragShader");
		constexpr uint64_t CUSTOM_SHADER = SliceEngine::FNVHash::fnv1a("CustomShader");
		constexpr uint64_t MATERIAL = SliceEngine::FNVHash::fnv1a("Material");
		constexpr uint64_t MODEL = SliceEngine::FNVHash::fnv1a("Model");
		constexpr uint64_t SKELETON = SliceEngine::FNVHash::fnv1a("Skeleton");
		constexpr uint64_t ANIMATION = SliceEngine::FNVHash::fnv1a("Animation");
		constexpr uint64_t SEQUENCEPACKAGE = SliceEngine::FNVHash::fnv1a("SequencePackage");
		constexpr uint64_t SEQUENCE = SliceEngine::FNVHash::fnv1a("Sequence");
		constexpr uint64_t SOUND = SliceEngine::FNVHash::fnv1a("Sound");
		constexpr uint64_t SCENE = SliceEngine::FNVHash::fnv1a("Scene");
		constexpr uint64_t PREFAB = SliceEngine::FNVHash::fnv1a("Prefab");
		constexpr uint64_t CONTROLLER = SliceEngine::FNVHash::fnv1a("Controller");
		constexpr uint64_t NAVMESH = SliceEngine::FNVHash::fnv1a("NavMesh");
		constexpr uint64_t NAVMESHBIN = SliceEngine::FNVHash::fnv1a("NavMeshBin");
		constexpr uint64_t CSV = SliceEngine::FNVHash::fnv1a("CSV");
		constexpr uint64_t FONT = SliceEngine::FNVHash::fnv1a("Font");

	}


	class MetaData
	{
	public:
		SliceEngine::GUID guid;
		std::string assetName;
		std::string assetType;
		std::string assetPath;
		std::string resourcePath;
		
		//MetaData() = default;
		virtual ~MetaData() = default;

		void InitMetaData(const std::filesystem::path path, AssetType type, const std::string& typeName)
		{
			std::filesystem::path mResourcesDirectory = std::filesystem::path("Resources");

			std::filesystem::path relativePath = std::filesystem::relative(path, "Assets");
		/*	switch (type) // comment out for now
		* // NOTE: No creating of new asset for skeleton or anim package anymore maybe
		* // NOTE: We store the resource reference in the meta data
		* // then the editor can retrieve it to show it in the content browser
			{
			case AssetType::Skeleton:
				assetName = path.stem().string() + "_skl";
				relativePath.replace_extension("")
				break;
			case AssetType::Animation:
				assetName = path.stem().string() + "_animpkg";
				break;
			}*/

			assetName = relativePath.generic_string();

			// changed to generate a random GUID based on time of creation
			guid = SliceEngine::GUID::Generate();
			assetType = typeName;
			assetPath = path.string();
			resourcePath = mResourcesDirectory.string() + "/" + std::to_string(guid.GetGUID()) + assetType;

		}

		virtual std::filesystem::path Serialize(const std::filesystem::path & ) = 0;
		virtual void Deserialize(const std::filesystem::path& desc_path)
		{
			std::ifstream inFile(desc_path);
			if (!inFile.is_open())
			{
				return;
			}

			nlohmann::json metaData;
			try
			{
				inFile >> metaData;
			}
			catch (nlohmann::json::parse_error&)
			{
				return;
			}

			if (metaData.contains("guid"))
				guid = SliceEngine::GUID(metaData["guid"].get<uint64_t>());

			if (metaData.contains("assetName"))
				assetName = metaData["assetName"].get<std::string>();

			if (metaData.contains("assetType"))
				assetType = metaData["assetType"].get<std::string>();

			if (metaData.contains("assetPath"))
				assetPath = metaData["assetPath"].get<std::string>();
			
			if (metaData.contains("resourcePath"))
				resourcePath = metaData["resourcePath"].get<std::string>();
		}
	};

	struct DefaultMeta : public MetaData
	{	
		std::filesystem::path Serialize(const std::filesystem::path&) override
		{
			return std::filesystem::path("");
		}

		void Deserialize(const std::filesystem::path& desc_path) override
		{
			std::ifstream inFile(desc_path);
			nlohmann::json metaData;

			if (!inFile.is_open())
			{
				SLICE_LOG_WARNING("File not found for Deserialisation!");
				return;
			}

			else
			{
				inFile >> metaData;
				inFile.close();
			}

			guid = SliceEngine::GUID(metaData["guid"].get<uint64_t>());
			assetName = metaData["assetName"].get<std::string>();
			assetType = metaData["assetType"].get<std::string>();
			assetPath = metaData["assetPath"].get<std::string>();
			resourcePath = metaData["resourcePath"].get<std::string>();
		}
	};

	struct TextureData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::TEXTURE;

		UsageType usage_type{ UsageType::Color };

		CompressionFormat cmp_format{ CompressionFormat::BC7 };
		bool is_srgb{ true };
		float comp_quality{ 1.f };

		bool generateMips{ true };
		MipMapFilter mip_filter{ MipMapFilter::BOX };
		unsigned char mip_count{ 8 };

		bool premultiply_alpha{ false };

		std::filesystem::path Serialize(const std::filesystem::path & desc_path) override
		{
			// now set the resource path
			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			// specific properties to texture goes here but we dh that yet
			// now we have specific properties :)
			metaJson["usage"] = usage_type;
			metaJson["compression"] = cmp_format;
			metaJson["srgb"] = is_srgb;
			metaJson["comp_quality"] = comp_quality;
			metaJson["generateMips"] = generateMips;
			metaJson["mip_filter"] = mip_filter;
			metaJson["mip_count"] = mip_count;
			metaJson["premultiply"] = premultiply_alpha;
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}

		void Deserialize(const std::filesystem::path& desc_path) override
		{
			std::ifstream inFile(desc_path);
			nlohmann::json metaData;

			if (!inFile.is_open())
			{
				SLICE_LOG_WARNING("File not found for Deserialisation!");
				return;
			}

			else
			{
				inFile >> metaData;
				inFile.close();
			}

			guid = SliceEngine::GUID(metaData["guid"].get<uint64_t>());
			assetName = metaData["assetName"].get<std::string>();
			assetType = metaData["assetType"].get<std::string>();
			assetPath = metaData["assetPath"].get<std::string>();
			resourcePath = metaData["resourcePath"].get<std::string>();

			usage_type = metaData.value<UsageType>("usage", UsageType::Color);
			cmp_format = metaData.value<CompressionFormat>("compression", CompressionFormat::BC3);
			is_srgb = metaData.value<bool>("srgb", true);
			comp_quality = metaData.value<float>("comp_quality", 1.f);
			generateMips = metaData.value<bool>("generateMips", false);
			mip_filter = metaData.value<MipMapFilter>("mip_filter", MipMapFilter::BOX);
			mip_count = metaData.value<unsigned char>("mip_count", 8);

			premultiply_alpha = metaData.value<bool>("premultiply", false);
		}
	};

	// blank for now cause we're using a default 36 vertice cube for testing
	// will implement later when we have a model format
	struct ModelData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MODEL;

		bool is_static{ false };
		SliceEngine::GUID skeletonGUID;
		SliceEngine::GUID animationGUID;

		// instead of meta path, itll be path to the resourcec
		// i kinda scared to rename cause removing it broke a bunch of shit for some reason
		// and idk if we'll ever need the path to resouce
		std::string skeleMetaPath{};
		std::string animMetaPath{};

		std::filesystem::path Serialize(const std::filesystem::path & desc_path) override
		{
			// now set the resource path
			//resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

			//uint64_t g = guid.GetGUID();
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			metaJson["static"] = is_static;
			metaJson["skeleMetaPath"] = skeleMetaPath;
			metaJson["animMetaPath"] = animMetaPath;

			metaJson["skeletonGUID"] = skeletonGUID;
			metaJson["animationGUID"] = animationGUID;
			// specific properties to model goes here but we dh that yet
			
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
		void Deserialize(const std::filesystem::path & desc_path) override
		{
			std::ifstream inFile(desc_path);
			nlohmann::json metaData;

			if (!inFile.is_open())
			{
				SLICE_LOG_WARNING("File not found for Deserialisation!");
				return;
			}

			else
			{
				inFile >> metaData;
				inFile.close();
			}

			guid = SliceEngine::GUID(metaData["guid"].get<uint64_t>());
			assetName = metaData["assetName"].get<std::string>();
			assetType = metaData["assetType"].get<std::string>();
			assetPath = metaData["assetPath"].get<std::string>();
			resourcePath = metaData["resourcePath"].get<std::string>();
			// small to do : pls dont make it crash
			is_static = metaData["static"].get<bool>();
			skeleMetaPath = metaData["skeleMetaPath"].get<std::string>();
			animMetaPath = metaData["animMetaPath"].get<std::string>();

			if (metaData.contains("skeletonGUID"))
				skeletonGUID = SliceEngine::GUID(metaData["skeletonGUID"].get<uint64_t>());
			if (metaData.contains("animationGUID"))
				animationGUID = SliceEngine::GUID(metaData["animationGUID"].get<uint64_t>());
		}
	};
	
	struct SkeletonData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SKELETON;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			//resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

			//uint64_t g = guid.GetGUID();
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			// specific properties to model goes here but we dh that yet

			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};

	struct AnimationData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::ANIMATION;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			//resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

			//uint64_t g = guid.GetGUID();
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			// specific properties to model goes here but we dh that yet

			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};

	struct SequencePkgData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SEQUENCEPACKAGE;

		std::vector<std::string> animations;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

			//uint64_t g = guid.GetGUID();
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			// specific properties to model goes here but we dh that yet

			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}

		void SerializeAsset(const std::filesystem::path& desc_path) 
		{
			nlohmann::json metaJson;
	
			metaJson["Animations"] = animations;


			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}
		}

		bool DeserializeAsset(const std::filesystem::path& filePath)
		{
			std::ifstream inFile{ filePath };
			if (inFile.fail())
			{
				return false;
			}

			nlohmann::json assetJson = nlohmann::json::parse(inFile);

			animations = assetJson["Animations"].get<std::vector<std::string>>();

			return true;
		}

		void LoadSequencePkgData(const SliceEngine::SliceEngineTypes::SequencePackage& newAnim)
		{
			for (const auto& anim : newAnim.animations)
			{
				animations.push_back(anim.name);
			}
		}
	};

	struct SequenceData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SEQUENCE;

		std::vector<std::pair<unsigned int, glm::vec3>> transforms{};
		std::vector<std::pair<unsigned int, glm::vec3>> rotation{};
		std::vector<std::pair<unsigned int, glm::vec3>> scale{};
		std::string name{};
		unsigned int fps{};
		float duration{};
		unsigned int num_frames{};

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

			//uint64_t g = guid.GetGUID();
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			// specific properties to model goes here but we dh that yet

			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}

		void SerializeAsset(const std::filesystem::path& desc_path)
		{

			nlohmann::json metaJson;

			metaJson["Name"] = name;
			metaJson["FPS"] = fps;
			metaJson["Duration"] = duration;
			metaJson["Number of Frames"] = num_frames;
			metaJson["Transforms"] = transforms;
			metaJson["Rotations"] = rotation;
			metaJson["Scales"] = scale;

			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}
		}

		bool DeserializeAsset(const std::filesystem::path& filePath)
		{
			std::ifstream inFile{ filePath };
			if (inFile.fail())
			{
				return false;
			}

			nlohmann::json assetJson = nlohmann::json::parse(inFile);

			name = assetJson["Name"];
			fps = assetJson["FPS"];
			duration = assetJson["Duration"];
			num_frames = assetJson["Number of Frames"];
			transforms = assetJson["Transforms"].get<std::vector<std::pair<unsigned int,glm::vec3>>>();
			rotation = assetJson["Rotations"].get<std::vector<std::pair<unsigned int,glm::vec3>>>();
			scale = assetJson["Scales"].get<std::vector<std::pair<unsigned int,glm::vec3>>>();

			return true;
		}

		void LoadSequenceData(const SliceEngine::SliceEngineTypes::Sequence& newAnim)
		{
			name = newAnim.name;
			fps = newAnim.fps;
			duration = newAnim.duration;
			num_frames = newAnim.num_frames;

			transforms = newAnim.transform;
			rotation = newAnim.rotation;
			scale = newAnim.scale;
		}

		void UnLoadSequenceData(SliceEngine::SliceEngineTypes::Sequence& newAnim)
		{
			newAnim.name = name;
			newAnim.fps = fps;
			newAnim.duration = duration;
			newAnim.num_frames = num_frames;

			newAnim.transform = transforms;
			newAnim.rotation = rotation;
			newAnim.scale = scale;
		}
	};

	struct SceneData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SCENE;

		std::string navMeshFile;
		SliceEngine::GUID navMeshGUID;

		std::string navMeshBinFile;
		SliceEngine::GUID navMeshBinGUID;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			// technically this is done in compiling of asset
			// but scene has no compiling so we just set it here
			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			// specific properties to scene goes here but we dh that yet
			metaJson["navMeshFile"] = navMeshFile;
			metaJson["navMeshGUID"] = navMeshGUID.GetGUID(); 
			metaJson["navMeshBinFile"] = navMeshBinFile;
			metaJson["navMeshBinGUID"] = navMeshBinGUID.GetGUID();
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};

	struct AudioData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SOUND;

		AudioStream stream{ AudioStream::CREATE_SAMPLE };
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{

		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			metaJson["stream"] = stream;

			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};

	struct PrefabData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::PREFAB;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			// technically this is done in compiling of asset
			// but scene has no compiling so we just set it here

		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			

			// specific properties to scene goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};

	struct CSVData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::CSV;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			// technically this is done in compiling of asset
			// but scene has no compiling so we just set it here

			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;


			// specific properties to scene goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};

	struct ShaderData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SHADER;
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to shader goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};
	struct CustomShaderData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::CUSTOM_SHADER;
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to shader goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
		void SerializeDefaultAsset(const std::filesystem::path& desc_path)
		{
			std::ofstream output(desc_path);

			std::string defaultTxt{ R"({
    "Main": {
        "END_COLOR": {
            "FinalNode1": [
                "Node30"
            ]
        },
        "END_EMISSION": {
            "FinalNode3": [
                "Node74"
            ]
        },
        "END_METALLIC": {
            "FinalNode0": [
                "Node42"
            ]
        },
        "END_NORMAL": {
            "FinalNode4": [
                "Node34"
            ]
        },
        "END_ROUGHNESS": {
            "FinalNode2": [
                "Node46"
            ]
        },
        "Flip_Y_Vec2": {
            "Node27": [
                "vUV"
            ]
        },
        "Multiply": {
            "Node30": [
                "color",
                "Node38"
            ],
            "Node74": [
                "Node545",
                "EmissionMult"
            ],
            "Node545": [
                "Node50",
                "color2"
            ]
        },
        "sampleTexture": {
            "Node34": [
                "NormalMap",
                "Node27"
            ],
            "Node38": [
                "albedo",
                "Node27"
            ],
            "Node42": [
                "MetallicMap",
                "Node27"
            ],
            "Node46": [
                "RoughnessMap",
                "Node27"
            ],
            "Node50": [
                "EmissionMap",
                "Node27"
            ]
        }
    },
    "Params": {
        "Bools": {},
        "Floats": {
            "EmissionMult": 1.0
        },
        "Ints": {},
        "Textures": {
            "EmissionMap": 15644028420850910905,
            "MetallicMap": 11697903386653029786,
            "NormalMap": 15800990347539648889,
            "RoughnessMap": 11697903386653029786,
            "albedo": 11697903386653029786
        },
        "Uints": {}
    }
})" };

			if (output.is_open())
			{
				output << defaultTxt;
				output.close();
			}
		}
	};
	struct VertShaderData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::VERT_SHADER;
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to shader goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};
	struct GeomShaderData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::GEOM_SHADER;
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to shader goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};
	struct FragShaderData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::FRAG_SHADER;
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to shader goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
	};
	
	struct MaterialData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MATERIAL;

		SliceEngine::GUID shader = (SliceEngine::GUID)0;
		//GUID normalMap;
		glm::vec4 color{ 1.0f };
		glm::vec4 color2{ 1.0f };
		bool isTranslucent{ false };
		bool isIgnoreLighting{ false };
		std::map<std::string, std::variant<bool, uint32_t, int32_t, float, SliceEngine::GUID>> data;
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			std::ifstream inFile{ desc_path };
			if (inFile.fail())
			{
				return;
			}

			nlohmann::json metaJson = nlohmann::json::parse(inFile);
			guid = (SliceEngine::GUID)metaJson["guid"].get<uint64_t>();
			assetName = metaJson["assetName"];
			assetType = metaJson["assetType"];
			assetPath = metaJson["assetPath"];
			resourcePath = metaJson["resourcePath"];

			inFile.close();
		}

		void DeserializeAsset(const std::filesystem::path& desc_path)
		{
			// now set the resource path
			std::ifstream inFile{ desc_path };
			if (inFile.fail())
			{
				return;
			}

			nlohmann::json metaJson = nlohmann::json::parse(inFile);
			// properties
			shader = (SliceEngine::GUID)metaJson["shader"].get<uint64_t>();
			auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			auto shdr = resourceMgr->get<SliceEngine::SliceEngineTypes::CustomShader>(shader);
			for (auto& i : shdr.get()->dataIn)
			{
				if (metaJson["data"].contains(i.name))
				{
					switch (i.dataType)
					{
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
					{
						bool b = metaJson["data"][i.name];
						data[i.name] = b;
						break;
					}
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::UINT:
					{
						uint32_t b = metaJson["data"][i.name];
						data[i.name] = b;
						break;
					}
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::INT:
					{
						int32_t b = metaJson["data"][i.name];
						data[i.name] = b;
						break;
					}
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
					{
						float b = metaJson["data"][i.name];
						data[i.name] = b;
						break;
					}
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::TEXTURE:
					{
						uint64_t b = metaJson["data"][i.name];
						data[i.name] = (SliceEngine::GUID)b;
						break;
					}
					}
				}
				else
				{
					switch (i.dataType)
					{
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
						data[i.name] = std::get<bool>(i.baseData);
						break;
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::UINT:
						data[i.name] = std::get<uint32_t>(i.baseData);
						break;
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::INT:
						data[i.name] = std::get<int32_t>(i.baseData);
						break;
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
						data[i.name] = std::get<float>(i.baseData);
						break;
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::TEXTURE:
						data[i.name] = (SliceEngine::GUID)std::get<uint64_t>(i.baseData);
						break;
					}
				}
			}

			from_json(metaJson["color"], color);
			if (metaJson.contains("color2"))
				from_json(metaJson["color2"], color2);
			if(metaJson.contains("translucency"))
				isTranslucent = metaJson["translucency"];
			if(metaJson.contains("ignoreLights"))
				isIgnoreLighting = metaJson["ignoreLights"];
			inFile.close();
		}

		void SerializeAsset(const std::filesystem::path& desc_path)
		{
			nlohmann::json metaJson;
			// specific properties to shader goes here but we dh that yet
			metaJson["shader"] = shader.GetGUID();
			to_json(metaJson["color"], color);
			to_json(metaJson["color2"], color2);
			metaJson["translucency"] = isTranslucent;
			metaJson["ignoreLights"] = isIgnoreLighting;
			nlohmann::json dataJson = nlohmann::json::object();
			for (const auto& [key, val] : data)
			{
				std::visit([&](auto&& arg) {
					if (std::holds_alternative<SliceEngine::GUID>(val))
						dataJson[key] = std::get<SliceEngine::GUID>(val).GetGUID();
					else
						dataJson[key] = arg;
				}, val);
			}
			metaJson["data"] = dataJson;

			std::ofstream output(desc_path);

			if (output.is_open())
			{
				output << metaJson.dump(4);
				output.close();
			}
		}
	};

	struct StateMachineData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::CONTROLLER;

		std::map<std::string, rttr::variant> parameters{};
		std::unordered_map<std::string, SliceEngine::SliceEngineTypes::State> stateMap{};
		glm::vec2 entryPosition{};
		glm::vec2 exitPosition{};
		std::string entryState{};

		StateMachineData() = default;
		~StateMachineData() = default;

		void to_json(nlohmann::json& j, const rttr::variant& var)
		{
			rttr::type type = var.get_type();

			if (type == rttr::type::get<int>()) {
				j = var.get_value<int>();
			}
			else if (type == rttr::type::get<float>()) {
				j = var.get_value<float>();
			}
			else if (type == rttr::type::get<double>()) {
				j = var.get_value<double>();
			}
			else if (type == rttr::type::get<bool>()) {
				j = var.get_value<bool>();
			}
			else if (type == rttr::type::get<std::string>()) {
				j = var.get_value<std::string>();
			}
			else {
				// Handle unknown types, e.g., serialize as null or throw
				j = nullptr;
			}
		}
		void to_json(nlohmann::json& j, const SliceEngine::SliceEngineTypes::Condition& c)
		{
			to_json(j["value"], c.value);
			j["paramName"] = c.paramName;
			j["comparisonOP"] = c.op;
		}
		void to_json(nlohmann::json& j, const SliceEngine::SliceEngineTypes::Transition& t)
		{
			j["sourceState"] = t.sourceState;
			j["targetState"] = t.targetState;
			j["hasExitTime"] = t.hasExitTime;
			j["exitTime"] = t.exitTime;
			j["entryTime"] = t.entryTime;
			for (const auto& it : t.conditions)
			{
				nlohmann::json tempTransJson;
				to_json(tempTransJson, it);
				j["conditions"].push_back(tempTransJson);

			}
		}
		void to_json(nlohmann::json& j, const SliceEngine::SliceEngineTypes::State& s)
		{
			j["stateName"] = s.stateName;
			j["currAnimIdx"] = s.curr_anim_idx;
			j["isLoop"] = s.isLoop;
			j["mNodePos"] = s.mNodePos;
			j["fps"] = s.fps;
			j["speed"] = s.animationSpeed;

			j["transitions"] = nlohmann::json::array();

			for (const auto& it : s.transitions)
			{
				nlohmann::json tempTransJson;

				// 4. Call your "working" Style 2 to_json to populate it
				to_json(tempTransJson, it);

				// 5. Add the populated object to the array
				j["transitions"].push_back(tempTransJson);

			}
		}
		void from_json(const nlohmann::json& j, rttr::variant& var)
		{
			if (j.is_number_integer()) {
				var = j.get<int>();
			}
			else if (j.is_number_float()) {
				var = j.get<float>();
			}
			else if (j.is_boolean()) {
				var = j.get<bool>();
			}
			else if (j.is_string()) {
				var = j.get<std::string>();
			}
			else {
				// Handle unknown or null types if necessary
				var = rttr::variant();
			}
		}
		void from_json(const nlohmann::json& j, SliceEngine::SliceEngineTypes::Condition& c)
		{
			j.at("paramName").get_to(c.paramName);
			j.at("comparisonOP").get_to(c.op);
			// Deserializing condition, which is an rttr::variant
			if (j.contains("value")) {
				from_json(j.at("value"), c.value);
			}
			else {
				c.value = rttr::variant();
			}
		}
		void from_json(const nlohmann::json& j, SliceEngine::SliceEngineTypes::Transition& t)
		{
			// A helper lambda to convert the string operator to enum
			j.at("sourceState").get_to(t.sourceState);
			j.at("targetState").get_to(t.targetState);
			j.at("hasExitTime").get_to(t.hasExitTime);
			j.at("exitTime").get_to(t.exitTime);
			j.at("entryTime").get_to(t.entryTime);

			t.conditions.clear();
			const auto& conditions_json = j.at("conditions");

			// Iterate over the array of transitions
			for (const auto& condition_json : conditions_json)
			{
				SliceEngine::SliceEngineTypes::Condition tmpCons;
				// Call the Transition deserialization helper
				from_json(condition_json, tmpCons);
				t.conditions.push_back(tmpCons);
			}
		}
		void from_json(const nlohmann::json& j, SliceEngine::SliceEngineTypes::State& s)
		{
			j.at("stateName").get_to(s.stateName);
			j.at("currAnimIdx").get_to(s.curr_anim_idx);
			j.at("isLoop").get_to(s.isLoop);
			j.at("mNodePos").get_to(s.mNodePos);
			j.at("fps").get_to(s.fps);
			//j.at("speed").get_to(s.animationSpeed);
			s.animationSpeed = j.value("speed", 1.0f);


			s.transitions.clear();
			const auto& transitions_json = j.at("transitions");

			// Iterate over the array of transitions
			for (const auto& transition_json : transitions_json)
			{
				SliceEngine::SliceEngineTypes::Transition tmpTrans;
				// Call the Transition deserialization helper
				from_json(transition_json, tmpTrans);
				s.transitions.push_back(tmpTrans);
			}
		}

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties
			metaJson["entryState"] = entryState;
			metaJson["entryNodePosition"] = entryPosition;
			metaJson["exitNodePosition"] = exitPosition;

			for (auto it : parameters)
			{
				to_json(metaJson["parameters"][it.first], it.second);
			}
			
			for (auto it : stateMap)
			{
				to_json(metaJson["stateMap"][it.first], it.second);
			}

			std::ofstream output(desc_path);
			if (output.is_open())
			{
				output << metaJson.dump(4);
				output.close();
			}

			return std::filesystem::path(desc_path);
		}

		void SerializeAsset(std::filesystem::path const path = std::filesystem::path{})
		{
			nlohmann::json assetJson;

			assetJson["entryState"] = entryState;
			assetJson["entryNodePosition"] = entryPosition;
			assetJson["exitNodePosition"] = exitPosition;
	
			nlohmann::json parametersJson;
			for (const auto& pair : parameters)
			{
				to_json(parametersJson[pair.first], pair.second);
			}
			assetJson["parameters"] = parametersJson;

			nlohmann::json stateMapJson;
			for (const auto& pair : stateMap)
			{
				to_json(stateMapJson[pair.first], pair.second);
			}
			assetJson["stateMap"] = stateMapJson;

			std::filesystem::path filepath = path;
			if (filepath.empty())
				filepath = assetPath;

			std::ofstream output(filepath);

			if (output.is_open())
			{
				output << assetJson.dump(4);
				output.close();
			}

			else
			{
				SLICE_LOG_ERROR("Error in opening file for writing: " +  assetPath);
			}
		}

		/// <summary>
		/// For creating the default player controller while editor is still being fixed
		/// </summary>
		/// <param name="desc_path"></param>
		void SerializePlayerAsset(const std::filesystem::path& desc_path)
		{
			nlohmann::json metaJson;

			metaJson["entryState"] = "Idle";

			rttr::variant tmpVar;
			tmpVar = false;

			to_json(metaJson["parameters"]["AirDashStart"], tmpVar);
			to_json(metaJson["parameters"]["Attack1"], tmpVar);
			to_json(metaJson["parameters"]["Attack1_Fast"], tmpVar);
			to_json(metaJson["parameters"]["AttackToIdle1"], tmpVar);
			to_json(metaJson["parameters"]["Attack2"], tmpVar);
			to_json(metaJson["parameters"]["Attack2_Fast"], tmpVar);
			to_json(metaJson["parameters"]["AttackToIdle2"], tmpVar);
			to_json(metaJson["parameters"]["Attack3"], tmpVar);
			to_json(metaJson["parameters"]["Attack3_Fast"], tmpVar);
			to_json(metaJson["parameters"]["Attack3ToLoco"], tmpVar);
			to_json(metaJson["parameters"]["Attack3_FastToLoco"], tmpVar);
			to_json(metaJson["parameters"]["BackDashStart"], tmpVar);
			to_json(metaJson["parameters"]["DashStart"], tmpVar);
			to_json(metaJson["parameters"]["Fall"], tmpVar);
			to_json(metaJson["parameters"]["Walk"], tmpVar);
			to_json(metaJson["parameters"]["JumpLoop"], tmpVar);
			to_json(metaJson["parameters"]["Land"], tmpVar);
			to_json(metaJson["parameters"]["Plunge"], tmpVar);
			to_json(metaJson["parameters"]["PlungeLand"], tmpVar);
			to_json(metaJson["parameters"]["PlungeToIdle"], tmpVar);
			to_json(metaJson["parameters"]["PlungeToWalk"], tmpVar);

			SliceEngine::SliceEngineTypes::State tmpState;
			SliceEngine::SliceEngineTypes::Transition tmpTran;
			SliceEngine::SliceEngineTypes::Condition tmpCon;


			// -------------------- Idle --------------------
			tmpState.stateName = "Idle";
			tmpState.isLoop = true;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Attack1";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;

			tmpCon.paramName = "Attack1";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Walk";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;

			tmpCon.paramName = "Walk";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "JumpLoop";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;

			tmpCon.paramName = "JumpLoop";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["Idle"], tmpState);
			tmpState.transitions.clear();

			// -------------------- JumpLoop --------------------
			tmpState.stateName = "JumpLoop";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Plunge";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 0.7f;

			tmpCon.paramName = "Plunge";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Land";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;

			tmpCon.paramName = "Land";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "DoubleJump";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "DoubleJump";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["JumpLoop"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Plunge --------------------
			tmpState.stateName = "Plunge";
			tmpState.isLoop = true;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "PlungeLand";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "PlungeLand";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["Plunge"], tmpState);
			tmpState.transitions.clear();

			// -------------------- PlungeLand --------------------
			tmpState.stateName = "PlungeLand";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "PlungeToWalk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "PlungeToWalk";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "PlungeToIdle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "PlungeToIdle";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["PlungeLand"], tmpState);
			tmpState.transitions.clear();

			// -------------------- PlungeToIdle --------------------
			tmpState.stateName = "PlungeToIdle";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Idle";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["PlungeToIdle"], tmpState);
			tmpState.transitions.clear();

			// -------------------- PlungeToWalk --------------------
			tmpState.stateName = "PlungeToWalk";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Walk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Walk";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["PlungeToWalk"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Walk --------------------
			tmpState.stateName = "Walk";
			tmpState.isLoop = true;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Attack1";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Attack1";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Idle";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Idle";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "JumpLoop";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "JumpLoop";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["Walk"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack1 --------------------
			tmpState.stateName = "Attack1";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "AttackToIdle1";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "AttackToIdle1";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Attack2";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Attack2";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["Attack1"], tmpState);
			tmpState.transitions.clear();

			// -------------------- AttackToIdle1 --------------------
			tmpState.stateName = "AttackToIdle1";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Idle";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Walk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Walk";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();


			to_json(metaJson["stateMap"]["AttackToIdle1"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack2 --------------------
			tmpState.stateName = "Attack2";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "AttackToIdle2";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "AttackToIdle2";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Attack3";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Attack3";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["Attack2"], tmpState);
			tmpState.transitions.clear();

			// -------------------- AttackToIdle2 --------------------
			tmpState.stateName = "AttackToIdle2";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Idle";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Walk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Walk";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();


			to_json(metaJson["stateMap"]["AttackToIdle2"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack3 --------------------
			tmpState.stateName = "Attack3";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Attack3ToLoco";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Attack3ToLoco";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["Attack3"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack3ToLoco --------------------
			tmpState.stateName = "Attack3ToLoco";
			tmpState.isLoop = false;

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;

			tmpCon.paramName = "Idle";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			tmpTran.sourceState = tmpState.stateName;
			tmpTran.targetState = "Walk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			
			tmpCon.paramName = "Walk";
			tmpCon.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.conditions.push_back(tmpCon);

			tmpState.transitions.push_back(tmpTran);
			tmpTran.conditions.clear();

			to_json(metaJson["stateMap"]["Attack3ToLoco"], tmpState);
			tmpState.transitions.clear();


			std::ofstream output(desc_path);

			if (output.is_open())
			{
				output << metaJson.dump(4);
				output.close();
			}
		}
	
		bool DeserializeAsset(const std::filesystem::path& filePath)
		{
			std::ifstream inFile{ filePath };
			if (inFile.fail())
			{
				return false;
			}

			nlohmann::json assetJson = nlohmann::json::parse(inFile);

			auto entry_pos = assetJson.find("entryNodePosition");
			auto exit_pos = assetJson.find("exitNodePosition");
			entryPosition = entry_pos != assetJson.end() ? entry_pos->get<glm::vec2>() : glm::vec2(0.0f);
			exitPosition = exit_pos != assetJson.end() ? exit_pos->get<glm::vec2>() : glm::vec2(0.0f);

			entryState = assetJson["entryState"];
			auto params = assetJson["parameters"];

			for (auto it = params.begin(); it != params.end(); ++it)
			{
				rttr::variant var;
				from_json(it.value(), var);
				parameters[it.key()] = var;
				//parameters[it.]
			}

			auto states = assetJson["stateMap"];

			for (auto it = states.begin(); it != states.end(); ++it)
			{
				SliceEngine::SliceEngineTypes::State state;
				from_json(it.value(), state);
				stateMap[it.key()] = state;
			}
			
			return true;
		}
	};

	struct NavMeshData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::NAVMESH;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
		resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties

			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}

	};

	struct NavMeshBinData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::NAVMESHBIN;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties

			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}

	};

	struct FontMetaData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::FONT;

		int font_resolution{50};
		int padding{ 2 };

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties
			metaJson["fontReso"] = font_resolution;
			metaJson["padding"] = padding;

			std::ofstream outFile(desc_path);
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path);
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
			std::ifstream inFile(desc_path);
			nlohmann::json metaData;

			if (!inFile.is_open())
			{
				SLICE_LOG_WARNING("File not found for Deserialisation!");
				return;
			}

			else
			{
				inFile >> metaData;
				inFile.close();
			}

			guid = SliceEngine::GUID(metaData["guid"].get<uint64_t>());
			assetName = metaData["assetName"].get<std::string>();
			assetType = metaData["assetType"].get<std::string>();
			assetPath = metaData["assetPath"].get<std::string>();
			resourcePath = metaData["resourcePath"].get<std::string>();
			font_resolution = metaData["fontReso"].get<int>();
			padding = metaData["padding"].get<int>();
		}
	};
}


#endif
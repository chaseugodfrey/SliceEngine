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
		Animation,
		Audio,
		Scene,
		Shader,
		Material,
		Prefab,
		Controller,
		NavMesh,
		Unsupported
	};
	enum CompressionFormat : std::uint8_t {
		//		RGBA_UNCOMPRESSED,
		RGB_BC1,
		RGBA_BC3
	};
	enum MipMapFilter : std::uint8_t {
		NONE,
		POINT,
		LINEAR,
		TRIANGLE,
		BOX
	};
	enum WrapType : std::uint8_t {
		CLAMP_TO_EDGE,
		WRAP,
		MIRROR
	};
	enum UsageType : std::uint8_t {
		COLOR,
		COLOR_ALPHA,
		TANGENT_NORMAL,
		INTENSITY
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
		constexpr uint64_t MATERIAL = SliceEngine::FNVHash::fnv1a("Material");
		constexpr uint64_t MODEL = SliceEngine::FNVHash::fnv1a("Model");
		constexpr uint64_t SKELETON = SliceEngine::FNVHash::fnv1a("Skeleton");
		constexpr uint64_t ANIMATION = SliceEngine::FNVHash::fnv1a("Animation");
		constexpr uint64_t SOUND = SliceEngine::FNVHash::fnv1a("Sound");
		constexpr uint64_t SCENE = SliceEngine::FNVHash::fnv1a("Scene");
		constexpr uint64_t PREFAB = SliceEngine::FNVHash::fnv1a("Prefab");
		constexpr uint64_t CONTROLLER = SliceEngine::FNVHash::fnv1a("Controller");
		constexpr uint64_t NAVMESH = SliceEngine::FNVHash::fnv1a("NavMesh");

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

			uint64_t typeID = 0;
			switch (type)
			{
			case AssetType::Texture:
				typeID = ResourceTypeIDs::TEXTURE;
				break;
			case AssetType::Model:
				typeID = ResourceTypeIDs::MODEL;
				break;
			case AssetType::Skeleton:
				typeID = ResourceTypeIDs::SKELETON;
				break;
			case AssetType::Animation:
				typeID = ResourceTypeIDs::ANIMATION;
				break;
			case AssetType::Audio:
				typeID = ResourceTypeIDs::SOUND;
				break;
			case AssetType::Scene:
				typeID = ResourceTypeIDs::SCENE;
				break;
			case AssetType::Shader:
				typeID = ResourceTypeIDs::SHADER;
				break;
			case AssetType::Prefab:
				typeID = ResourceTypeIDs::PREFAB;
				break;
			case AssetType::Controller:
				typeID = ResourceTypeIDs::CONTROLLER;
				break;
			case AssetType::NavMesh:
				typeID = ResourceTypeIDs::NAVMESH;
				break;
			case AssetType::Material:
				typeID = ResourceTypeIDs::MATERIAL;
				break;
			}


			assetName = path.stem().string();
			guid = SliceEngine::GUID::Generate(assetName, typeID);
			assetType = typeName;
			assetPath = path.string();
			resourcePath = mResourcesDirectory.string() + "/" + std::to_string(guid.GetGUID()) + assetType;

		}

		virtual std::filesystem::path Serialize(const std::filesystem::path & ) = 0;
		virtual void Deserialize(const std::filesystem::path & ) = 0;
	};

	struct TextureData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::TEXTURE;

		CompressionFormat cmp_format{ CompressionFormat::RGBA_BC3 };
		MipMapFilter mip_filter{ MipMapFilter::NONE };
		WrapType u_wrap{ WrapType::CLAMP_TO_EDGE };
		WrapType v_wrap{ WrapType::CLAMP_TO_EDGE };
		UsageType usage_type{ UsageType::COLOR };

		float comp_quality{ 0.5f };
		bool generateMips{ true };
		unsigned char mip_count{ 8 };
		bool hasAlpha{ true };
		unsigned char alpha_threshold{ 128 };	//used only for non-blending

		std::filesystem::path Serialize(const std::filesystem::path & desc_path) override
		{
			// now set the resource path
			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			// specific properties to texture goes here but we dh that yet
			// now we have specific properties :)
			metaJson["comp_format"] = cmp_format;
			metaJson["mip_filter"] = mip_filter;
			metaJson["u_wrap"] = u_wrap;
			metaJson["v_wrap"] = v_wrap;
			metaJson["comp_quality"] = comp_quality;
			metaJson["generateMips"] = generateMips;
			metaJson["mip_count"] = mip_count;
			metaJson["hasAlpha"] = hasAlpha;
			metaJson["alpha_threshold"] = alpha_threshold;
			// now create the meta file
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
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
			cmp_format = metaData["comp_format"].get<CompressionFormat>();
			mip_filter = metaData["mip_filter"].get<MipMapFilter>();
			u_wrap = metaData["u_wrap"].get<WrapType>();
			v_wrap = metaData["v_wrap"].get<WrapType>();
			comp_quality = metaData["comp_quality"].get <float> ();
			alpha_threshold = metaData["alpha_threshold"].get <char> ();
			generateMips = metaData["generateMips"].get <bool> ();
			hasAlpha = metaData["hasAlpha"].get <bool> ();
		}
	};

	// blank for now cause we're using a default 36 vertice cube for testing
	// will implement later when we have a model format
	struct ModelData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MODEL;

		bool is_static{ true };
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

			// specific properties to model goes here but we dh that yet
			
			// now create the meta file
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
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
			is_static = metaData["static"].get<bool>();
			skeleMetaPath = metaData["skeleMetaPath"].get<std::string>();
			animMetaPath = metaData["animMetaPath"].get<std::string>();
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
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
		}
	};

	struct AnimData : public MetaData
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
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
		}
	};

	struct SceneData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SCENE;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			// technically this is done in compiling of asset
			// but scene has no compiling so we just set it here
			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to scene goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
		}
	};

	struct AudioData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SOUND;

		AudioStream stream{ AudioStream::CREATE_SAMPLE };
		
		

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{

			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;

			metaJson["stream"] = stream;
			


			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");

			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}

		
		void Deserialize(const std::filesystem::path& desc_path) override
		{

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

			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			

			// specific properties to scene goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
		}
	};

	struct ShaderData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SHADER;
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to shader goes here but we dh that yet
			// now create the meta file
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
		}
	};
	
	struct MaterialData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MATERIAL;

		SliceEngine::GUID albedo = (SliceEngine::GUID)0;
		//GUID normalMap;
		float roughness = 0.0f;
		float metallic = 0.0f;
		glm::vec3 color{ 1.0f };
		
		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties to shader goes here but we dh that yet
			metaJson["albedo"] = albedo.GetGUID();
			metaJson["roughness"] = roughness;
			metaJson["metallic"] = metallic;
			to_json(metaJson["color"], color);

			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
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

			// properties
			roughness = metaJson["roughness"].get<float>();
			metallic = metaJson["metallic"].get<float>();
			from_json(metaJson["color"], color);
			albedo = (SliceEngine::GUID)metaJson["albedo"].get<uint64_t>();

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
			roughness = metaJson["roughness"].get<float>();
			metallic = metaJson["metallic"].get<float>();
			from_json(metaJson["color"], color);
			albedo = (SliceEngine::GUID)metaJson["albedo"].get<uint64_t>();

			inFile.close();
		}

		void SerializeAsset(const std::filesystem::path& desc_path)
		{
			nlohmann::json metaJson;
			// specific properties to shader goes here but we dh that yet
			metaJson["albedo"] = albedo.GetGUID();
			metaJson["roughness"] = roughness;
			metaJson["metallic"] = metallic;
			to_json(metaJson["color"], color);

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

		std::map<std::string, rttr::variant> parameters;
		std::unordered_map<std::string, SliceEngine::SliceEngineTypes::State> stateMap;
		std::string entryState;

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
		void to_json(nlohmann::json& j, const SliceEngine::SliceEngineTypes::Transition& t)
		{
			j["targetState"] = t.targetState;
			to_json(j["condition"], t.condition);
			j["parameterName"] = t.parameterName;
			j["comparisonOP"] = t.operation;
			j["hasExitTime"] = t.hasExitTime;
			j["exitTime"] = t.exitTime;
			j["entryTime"] = t.entryTime;
		}
		void to_json(nlohmann::json& j, const SliceEngine::SliceEngineTypes::State& s)
		{
			j["stateName"] = s.stateName;
			j["currAnimIdx"] = s.curr_anim_idx;
			j["isLoop"] = s.isLoop;
			j["mNodePos"] = s.mNodePos;

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

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			// now set the resource path
			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties
			metaJson["entryState"] = entryState;

			for (auto it : parameters)
			{
				to_json(metaJson["parameters"][it.first], it.second);
			}
			
			for (auto it : stateMap)
			{
				to_json(metaJson["stateMap"][it.first], it.second);
			}

			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}

		void Deserialize(const std::filesystem::path& desc_path) override
		{

		}

		void SerializeAsset(const std::filesystem::path& desc_path)
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


			// -------------------- Idle --------------------
			tmpState.stateName = "Idle";
			tmpState.isLoop = true;

			tmpTran.targetState = "Attack1";
			tmpTran.parameterName = "Attack1";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Walk";
			tmpTran.parameterName = "Walk";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "JumpLoop";
			tmpTran.parameterName = "JumpLoop";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["Idle"], tmpState);
			tmpState.transitions.clear();

			// -------------------- JumpLoop --------------------
			tmpState.stateName = "JumpLoop";
			tmpState.isLoop = false;

			tmpTran.targetState = "Plunge";
			tmpTran.parameterName = "Plunge";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 0.7f;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Land";
			tmpTran.parameterName = "Land";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "DoubleJump";
			tmpTran.parameterName = "DoubleJump";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["JumpLoop"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Plunge --------------------
			tmpState.stateName = "Plunge";
			tmpState.isLoop = true;

			tmpTran.targetState = "PlungeLand";
			tmpTran.parameterName = "PlungeLand";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["Plunge"], tmpState);
			tmpState.transitions.clear();

			// -------------------- PlungeLand --------------------
			tmpState.stateName = "PlungeLand";
			tmpState.isLoop = false;

			tmpTran.targetState = "PlungeToWalk";
			tmpTran.parameterName = "PlungeToWalk";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "PlungeToIdle";
			tmpTran.parameterName = "PlungeToIdle";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["PlungeLand"], tmpState);
			tmpState.transitions.clear();

			// -------------------- PlungeToIdle --------------------
			tmpState.stateName = "PlungeToIdle";
			tmpState.isLoop = false;

			tmpTran.targetState = "Idle";
			tmpTran.parameterName = "Idle";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["PlungeToIdle"], tmpState);
			tmpState.transitions.clear();

			// -------------------- PlungeToWalk --------------------
			tmpState.stateName = "PlungeToWalk";
			tmpState.isLoop = false;

			tmpTran.targetState = "Walk";
			tmpTran.parameterName = "Walk";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["PlungeToWalk"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Walk --------------------
			tmpState.stateName = "Walk";
			tmpState.isLoop = true;

			tmpTran.targetState = "Attack1";
			tmpTran.parameterName = "Attack1";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Idle";
			tmpTran.parameterName = "Idle";
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "JumpLoop";
			tmpTran.parameterName = "JumpLoop";
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpTran.hasExitTime = false;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["Walk"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack1 --------------------
			tmpState.stateName = "Attack1";
			tmpState.isLoop = false;

			tmpTran.targetState = "AttackToIdle1";
			tmpTran.parameterName = "AttackToIdle1";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Attack2";
			tmpTran.parameterName = "Attack2";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["Attack1"], tmpState);
			tmpState.transitions.clear();

			// -------------------- AttackToIdle1 --------------------
			tmpState.stateName = "AttackToIdle1";
			tmpState.isLoop = false;

			tmpTran.targetState = "Idle";
			tmpTran.parameterName = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Walk";
			tmpTran.parameterName = "Walk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);


			to_json(metaJson["stateMap"]["AttackToIdle1"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack2 --------------------
			tmpState.stateName = "Attack2";
			tmpState.isLoop = false;

			tmpTran.targetState = "AttackToIdle2";
			tmpTran.parameterName = "AttackToIdle2";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Attack3";
			tmpTran.parameterName = "Attack3";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["Attack2"], tmpState);
			tmpState.transitions.clear();

			// -------------------- AttackToIdle2 --------------------
			tmpState.stateName = "AttackToIdle2";
			tmpState.isLoop = false;

			tmpTran.targetState = "Idle";
			tmpTran.parameterName = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Walk";
			tmpTran.parameterName = "Walk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);


			to_json(metaJson["stateMap"]["AttackToIdle2"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack3 --------------------
			tmpState.stateName = "Attack3";
			tmpState.isLoop = false;

			tmpTran.targetState = "Idle";
			tmpTran.parameterName = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["Attack3"], tmpState);
			tmpState.transitions.clear();

			// -------------------- Attack3ToLoco --------------------
			tmpState.stateName = "Attack3ToLoco";
			tmpState.isLoop = false;

			tmpTran.targetState = "Idle";
			tmpTran.parameterName = "Idle";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			tmpTran.targetState = "Walk";
			tmpTran.parameterName = "Walk";
			tmpTran.hasExitTime = true;
			tmpTran.entryTime = 0.0f;
			tmpTran.exitTime = 1.0f;
			tmpTran.operation = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpState.transitions.push_back(tmpTran);

			to_json(metaJson["stateMap"]["Attack3ToLoco"], tmpState);
			tmpState.transitions.clear();


			std::ofstream output(desc_path);

			if (output.is_open())
			{
				output << metaJson.dump(4);
				output.close();
			}
		}
	};

	struct NavMeshData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::NAVMESH;

		std::filesystem::path Serialize(const std::filesystem::path& desc_path) override
		{
			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;
			nlohmann::json metaJson;
			metaJson["guid"] = guid.GetGUID();
			metaJson["assetName"] = assetName;
			metaJson["assetType"] = assetType;
			metaJson["assetPath"] = assetPath;
			metaJson["resourcePath"] = resourcePath;
			// specific properties

			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}

			return std::filesystem::path(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
		}


		void Deserialize(const std::filesystem::path& desc_path) override
		{

		}
	};
}


#endif
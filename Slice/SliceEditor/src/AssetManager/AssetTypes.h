#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

#include <filesystem>
#include "../../SliceEngine/src/Resource/ResourceManager.h"

namespace SliceEditor
{
	enum class AssetType
	{
		Texture,
		Model,
		Audio,
		Scene,
		Shader
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

	// type UUIDs 
	namespace ResourceTypeIDs
	{
		constexpr uint64_t TEXTURE = SliceEngine::FNVHash::fnv1a("Texture");
		constexpr uint64_t SHADER = SliceEngine::FNVHash::fnv1a("Shader");
		constexpr uint64_t MODEL = SliceEngine::FNVHash::fnv1a("Model");
		constexpr uint64_t SOUND = SliceEngine::FNVHash::fnv1a("Sound");
		constexpr uint64_t SCENE = SliceEngine::FNVHash::fnv1a("Scene");
	}

	class MetaData
	{
	public:
		SliceEngine::GUID guid;
		std::string assetName;
		std::string assetType;
		std::string assetPath;
		std::string resourcePath;

		virtual void Serialize(const std::filesystem::path & ) = 0;
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

		void Serialize(const std::filesystem::path & desc_path) override
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

			// now create the meta file
			std::ofstream outFile(desc_path.string() + "/" + std::to_string(guid.GetGUID()) + ".meta");
			if (outFile.is_open())
			{
				outFile << metaJson.dump(4);
				outFile.close();
			}
		}

		void Deserialize(const std::filesystem::path & desc_path) override
		{

		}
	};

	// blank for now cause we're using a default 36 vertice cube for testing
	// will implement later when we have a model format
	struct ModelData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MODEL;

		void Serialize(const std::filesystem::path & desc_path) override
		{
			// now set the resource path
			resourcePath = desc_path.string() + "/" + std::to_string(guid.GetGUID()) + assetType;

			nlohmann::json metaJson;

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
		}
		void Deserialize(const std::filesystem::path & desc_path) override
		{
		}
	};

	struct SceneData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SCENE;

		void Serialize(const std::filesystem::path& desc_path) override
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
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
		}
	};

	struct ShaderData : public MetaData
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SHADER;
		
		void Serialize(const std::filesystem::path& desc_path) override
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
		}
		void Deserialize(const std::filesystem::path& desc_path) override
		{
		}
	};
}


#endif
#ifndef RENDER_COMMAND_MANAGER_H
#define RENDER_COMMAND_MANAGER_H

#include <memory>
#include "../ECS/ECSTypes.h"
#include "../ECS/GameObject.h"

#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"

namespace SliceEngine
{
	class RenderCmdManager
	{
		using RCK_Size = uint64_t;		// long long
		using RCK_DepthT = uint32_t;	// int
		using RCK_ModelT = uint16_t;	// short
		const unsigned char RCK_ModelOffset = 40;
		// Reminder: 1byte = 2 Hex = 8 Bits
		enum RenderCmdKey : RCK_Size
		{
			// 4 outcomes - Opaque, Transcluscent, Additive, Subtractive
			MRCK_OPAQUE			= 0xC000'0000'0000'0000,
			MRCK_TRANSCLUCENT	= 0x8000'0000'0000'0000,
			MRCK_ADDICTIVE		= 0x4000'0000'0000'0000,
			MRCK_SUBTRACTIVE	= 0x0000'0000'0000'0000,
			MRCK_TRANSLUCENCY	= 0xC000'0000'0000'0000,
			// ------------------------ Normal ------------------
			MRCK_SHADER			= 0x3FC0'0000'0000'0000, // 255 Shader IDs
			MRCK_MODEL			= 0x003F'FF00'0000'0000, // 16'383 Models
			MRCK_SHADER_SET		= 0x0000'00FF'0000'0000, // Use the unsigned char enum GPUSetting
			MRCK_DEPTH_SORT		= 0x0000'0000'FFFF'FFFF
		};
#pragma region Instance Data
		struct ShadowInstanceData
		{
			glm::mat4 mdlMtx;
			//---
			unsigned int entityID;
			unsigned int padding0;
			unsigned int padding1;
			unsigned int padding2;
		};
		struct InstanceData
		{
			glm::mat4 mdlMtx;
			//---
			glm::vec4 color;
			//---
			float roughness;
			float metallic;
			unsigned int entityID;
			unsigned int texID;
		};
#pragma endregion
#pragma region Model Details
		struct ModelBasic
		{
			uint64_t mdl;
			unsigned char meshOffset;
			bool isSkin; // --TODO-- Cuz currently skin is based on if model isStatic, so i count this as a mdl feature
		};
		struct MdlFinder
		{
			uint64_t model;
			unsigned char meshOffset;
			const bool operator<(const MdlFinder& o) const
			{
				if (model < o.model)
					return true;
				if (model > o.model)
					return false;
				if (meshOffset < o.meshOffset)
					return true;
				if (meshOffset > o.meshOffset)
					return false;
				return false;
			}
			const bool operator==(const MdlFinder& o) const
			{
				if (model != o.model)
					return false;
				if (meshOffset != o.meshOffset)
					return false;
				return true;
			}
		};
#pragma endregion
	public:

		enum class DrawType : unsigned char
		{
			DRAW_MODELS,
			DRAW_OPAQUE,
			DRAW_TRANSLUCENT,
			DRAW_PREFAB_OPAQUE,
			DRAW_PREFAB_TRANSLUCENT
		};

		RenderCmdManager();
		~RenderCmdManager();
		void GatherDrawCalls();
		void SetVP(glm::mat4& V, glm::mat4& P);
		void SortTranslucent(Entity camEntity);
		void UseDrawCalls(GLuint mShader, DrawType drawType);
		void SingleDraw(GLuint mShader, const Entity& entity, DrawType drawType);
	private:
		RCK_ModelT GetModelDetails(uint64_t mdlID, unsigned char meshOffset, bool isSkin);
		void SetModelSkinUniform(GLuint mShader, bool isSkin, unsigned int entityID);
		unsigned int GetTextureDetails(GLuint64 bindlessID);

		const int mMaxInstance = 255;
		const float minDistTranslucent = -1.f;
		GLuint mBasicVBO;
		GLuint mDefaultVBO;
		GLuint mTextureVBO;
		glm::mat4 VP{};
		std::map<RCK_Size, std::vector<InstanceData>> renderCmds;
		std::vector<std::pair<RCK_Size,InstanceData>> translucentCmds; //single draw calls
		std::map<RCK_Size, std::vector<InstanceData>> prefabRenderCmds;
		std::vector<std::pair<RCK_Size,InstanceData>> prefabTranslucentCmds;
		std::map<RCK_ModelT, std::vector<ShadowInstanceData>> shadowRenderCmds;
		std::vector<ModelBasic> modelReferences;
		std::map<MdlFinder, RCK_ModelT> modelToIdx;
		std::vector<GLuint64> textureList;
		std::map<GLuint64, unsigned int> textureLoaded;
	};
}
#endif
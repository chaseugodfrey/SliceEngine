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
		using RCK_Size = uint64_t;
		using RCK_ModelT = uint16_t;
		const unsigned char RCK_ModelOffset = 38;// Each Hex +4
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
			MRCK_MODEL			= 0x003F'FFC0'0000'0000, // 65'535 Models
			MRCK_SHADER_SET		= 0x0000'003F'C000'0000, // Use the unsigned char enum GPUSetting
			// ------------------------ Extraction ------------------
			MRCK_EXTRACT_SHORT	= 0x0000'0000'0000'FFFF,
			MRCK_EXTRACT_CHAR	= 0x0000'0000'0000'00FF
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
		RenderCmdManager();
		~RenderCmdManager();
		void GatherDrawCalls();
		void UseDrawCalls(GLuint mShader, bool isForShadows);
		void SingleDraw(GLuint mShader, const Entity& entity, bool isForShadow);
	private:
		RCK_ModelT GetModelDetails(uint64_t mdlID, unsigned char meshOffset, bool isSkin);
		void SetModelSkinUniform(GLuint mShader, bool isSkin, unsigned int entityID);
		unsigned int GetTextureDetails(GLuint64 bindlessID);

		const int mMaxInstance = 255;
		GLuint mBasicVBO;
		GLuint mDefaultVBO;
		GLuint mTextureVBO;
		std::map<RCK_Size, std::vector<InstanceData>> renderCmds;
		std::vector<std::pair<RCK_Size,InstanceData>> translucentCmds;
		std::map<RCK_ModelT, std::vector<ShadowInstanceData>> shadowRenderCmds;
		std::vector<ModelBasic> modelReferences;
		std::map<MdlFinder, RCK_ModelT> modelToIdx;
		std::vector<GLuint64> textureList;
		std::map<GLuint64, unsigned int> textureLoaded;
	};
}
#endif
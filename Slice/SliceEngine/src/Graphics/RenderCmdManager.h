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
		//enum MainRenderCmdKey : unsigned long
		//{
		//	MRCK_TRANSLUCENCY	= 0xC000'0000'0000'0000, // 4 outcomes - Opaque, Transcluscent, Additive, Subtractive
		//	MRCK_DEPTH			= 0x0000'FFFF'0000'0000, // 65'535 Depth sorted
		//	MRCK_SHADER			= 0x0000'0000'FFFF'0000, // 65'535 Shader IDs
		//	MRCK_SHADER_SET		= 0x0000'0000'0000'FFFF  // Use the unsigned char enum GPUSetting
		//};
#pragma region Normal Render Commands
		enum RenderCmdIDParts : unsigned char
		{
			RCK_SHADER = 0,
			RCK_MODEL,
			RCK_MESH_OFFSET,// --TODO-- Assumes things w/ offset = Skin (actl, dun need care i think?)
			RCK_MAXBITS
		};

		struct RenderCmdID
		{
			uint64_t dat[RCK_MAXBITS];

			RenderCmdID(unsigned char num, ...)
			{
				va_list args;
				va_start(args, num);
				for (unsigned char i{}; i < num; ++i)
					dat[i] = va_arg(args, uint64_t);
				va_end(args);
			}

			const bool operator<(const RenderCmdID& o) const
			{
				for (unsigned char i{}; i < RCK_MAXBITS; ++i)
				{
					if (dat[i] < o.dat[i])
						return true;
					if (dat[i] > o.dat[i])
						return false;
				}
				return false;
			}

			const bool operator==(const RenderCmdID& o) const
			{
				for (unsigned char i{}; i < RCK_MAXBITS; ++i)
				{
					if (dat[i] != o.dat[i])
						return false;
				}
				return true;
			}
		};

		struct InstanceData
		{
			glm::mat4 mdlMtx;
			glm::vec4 color;
			float roughness;
			float metallic;
			uint64_t albedoTextureHandle;
			unsigned int entityID;
		};

		struct RenderBatch
		{
			std::vector<InstanceData> instances;

			Handle<SliceEngineTypes::Model> mdl;
		};
#pragma endregion
#pragma region Shadow Render Commands
		struct ShadowRenderCmdID
		{
			uint64_t model;
			unsigned int meshOffset;
			ShadowRenderCmdID(uint64_t mdl, unsigned int offset) : model(mdl), meshOffset(offset) {}

			const bool operator<(const ShadowRenderCmdID& o) const
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

			const bool operator==(const ShadowRenderCmdID& o) const
			{
				if (model != o.model)
					return false;
				if (meshOffset != o.meshOffset)
					return false;
				return true;
			}
		};
		struct ShadowInstanceData
		{
			glm::mat4 mdlMtx;
			unsigned int entityID;
		};
		struct ShadowRenderBatch
		{
			std::vector<ShadowInstanceData> instances;

			Handle<SliceEngineTypes::Model> mdl;
		};
#pragma endregion
	public:
		void GatherDrawCalls();
		void UseDrawCalls(GLuint mShader, bool isForShadows);

	private:
		void BasicDrawSettings(GLuint mShader, glm::mat4& mdlMtx, unsigned int meshOffset, unsigned int entityID);

		std::map<RenderCmdID, RenderBatch> renderCmds;
		std::map<ShadowRenderCmdID, ShadowRenderBatch> shadowRenderCmds;
	};
}
#endif
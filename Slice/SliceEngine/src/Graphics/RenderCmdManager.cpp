#include <pch.h>
#include "RenderCmdManager.h"
#include <glm/glm.hpp>
#include <glm/common.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Core/Core.h"

#include "TransformHelper.h"
#include "WorldSpaceGraphicsSystem.h"
#include "CameraSystem.h"
#include "LightingSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Systems/ParticleSystemManager.h"
#include "../Graphics/RenderManager.h" // --TODO-- Sus
#include "Systems/PrefabSystem.h"
#include "Systems/SceneSystem.h"

#include "Resource/Shader.h"
#include "Resource/Model.h"

// SSBOs
// Textures			  0 - Custom Shaders (Deferred.frag)
// Mat4,eID,texID,col 1 - Instanced.vert, Deferred.vert, debugOutline.vert, shadow.vert, pointShadow.vert (Deprecated: Deferred.frag)
// uvec4			  2 - Custom Shaders (Deferred.frag)
// Font				  3 - uiFont.vert

// UBOs
// Mat4[16]			  0 - Lighting.frag, Shadow.geom


namespace SliceEngine
{
	RenderCmdManager::RenderCmdManager()
	{
		auto* eventManager = EventManager::GetInstance();
		eventManager->Subscribe<OnPlayEvent, &RenderCmdManager::HandlePlayEvent>(this);

		glCreateBuffers(1, &mTextureVBO);
		glNamedBufferStorage(mTextureVBO, mMaxInstance * sizeof(GLuint64), NULL, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mTextureVBO);
		
		mBasicIMtx.resize(mMaxInstance);
		glCreateBuffers(1, &mIVBO);
		glNamedBufferStorage(mIVBO, sizeof(BasicIDat) * mBasicIMtx.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mIVBO);

		glCreateBuffers(1, &mEVBO);
		glNamedBufferStorage(mEVBO, sizeof(glm::uvec4) * mMaxInstance * mEVBOSafetyMult, nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mEVBO);
	}
	RenderCmdManager::~RenderCmdManager()
	{
		glDeleteBuffers(1, &mTextureVBO);
		glDeleteBuffers(1, &mIVBO);
		glDeleteBuffers(1, &mEVBO);
	}

	void RenderCmdManager::HandlePlayEvent(const OnPlayEvent& event)
	{
		// Particles don't have instanced Materials
		// Copy all materials
		auto view = Core::GetInstance()->GetRegistry().view<renderEntity>();
		for (auto entity : view)
		{
			auto& rend = Core::GetInstance()->GetRegistry().get<Renderer>(entity);
			rend.materialInstance = *(rend.materialHandle.get());
		}
	}

	void RenderCmdManager::GatherDrawCalls()
	{
		renderCmds.clear();
		shadowRenderCmds.clear();
		translucentCmds.clear();
		prefabRenderCmds.clear();
		prefabTranslucentCmds.clear();
		lastShadowOffset = lastRenderOffset = lastRenderPrefabOffset = lastTranslucentOffset = lastTranslucentPrefabOffset = glm::vec3(0, 0, 0);

		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<renderEntity>(entt::exclude<InactiveEntity>); // renderEntity // visibleEntity
		//sScene->mCurrentState;
		for (auto entity : view)
		{
			auto& rend = core->GetRegistry().get<Renderer>(entity);
			auto model = rend.modelHandle;
			if (!model.IsValid()) return;
			const SliceEngine::SliceEngineTypes::Material* material;
			if (core->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE)
			{
				if (!rend.materialInstance.albedo.IsValid() || !rend.materialInstance.shader.IsValid())
					rend.materialInstance = *(rend.materialHandle.get());
				material = &rend.materialInstance;
			}
			else
				material = rend.materialHandle.get();

			auto* rcmds = &renderCmds;
			auto* rtcmds = &translucentCmds;
			bool isPrefab = Core::GetInstance()->mFactory.mRegistry.any_of<PrefabEditingEntity>(entity);
			if (isPrefab)
			{
				rcmds = &prefabRenderCmds;
				rtcmds = &prefabTranslucentCmds;
			}

			//uint64_t shaderID = 9461939409271178249;// --TODO-- Should be responsibility of material
			RCK_ModelT mdlDet = GetModelDetails(model.getGUID().GetGUID(), rend.meshOffset, rend.skinned && !model.get()->is_static);

			uint8_t shdDet = GetShaderDetails(material->shader.get()->s);


			RCK_Size key = (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset) | (static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset); // as long as number dun hit that high, shouldn't overload
			BasicIDat data;
			if (material->color.a > 0.999f)
			{
				key = key | MRCK_OPAQUE;
				SetColor(data, glm::vec4(material->color.r, material->color.g, material->color.b, 1.f));
			}
			else
			{
				key = key | MRCK_TRANSCLUCENT;
				SetColor(data, material->color);
			}
			
			data.mdlMtx = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity).transform;
			data.texID = GetTextureDetails(material->albedo.get()->bindless_id);
			data.entityID = static_cast<unsigned int>(entity);

			if (rend.castShadow && !isPrefab)
				shadowRenderCmds[mdlDet].push_back(data);

			if ((key & MRCK_TRANSLUCENCY) == MRCK_TRANSCLUCENT)
			{
				TranslucentCmd tc{ key, data };
				SingleExtAppend(tc.ext, material);
				(*rtcmds).emplace_back(tc);
			}
			else
			{
				AppendRenderCmd((*rcmds)[key], data, material);
				(*rcmds)[key].numVar = static_cast<uint32_t>(material->shader.get()->dataIn.size());
			}
		}
	
		// Gather Particles --TODO-- Gather shader for particles too
		for (auto& ptx : Core::GetInstance()->GetSystem<ParticleSystemManager>().particlesTransforms)
		{
			if (!ptx.isMeshParticle)
			{
				auto model = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT);

				RCK_ModelT mdlDet = GetModelDetails(model.getGUID().GetGUID(), 0, false);
				// --TODO-- Currently hard set particles shader
				uint8_t shdDet = GetShaderDetails(Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::CustomShader>("CustomShader/particles.cshader").get()->s);
				RCK_Size key =
					(static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset) |
					(static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset); // as long as number dun hit that high, shouldn't overload
				if (ptx.colour.a > 0.999f)
					key = key | MRCK_OPAQUE;
				else
					key = key | MRCK_TRANSCLUCENT;

				BasicIDat data;
				data.mdlMtx = ptx.transform;
				SetColor(data, ptx.colour);
				data.texID = GetTextureDetails(ptx.textureID);
				data.entityID = 0;

				//shadowRenderCmds[mdlDet].emplace_back(ShadowInstanceData(data.mdlMtx));

				if ((key & MRCK_TRANSLUCENCY) == MRCK_TRANSCLUCENT)
				{
					TranslucentCmd tc{ key, data };
					translucentCmds.emplace_back(tc);
				}
				else
				{
					SetAlpha(data, 1.f);
					// --TODO--
					renderCmds[key].base.push_back(std::move(data));
				}
			}
			else 
			{
				const GUID& modelGUID = ptx.modelGUID;
				const GUID& materialGUID = ptx.materialGUID;

				if (!modelGUID.IsValid() || !materialGUID.IsValid())
					continue;

				auto* model = Core::GetInstance()
					->GetResourceManager()
					->get<SliceEngineTypes::Model>(modelGUID)
					.get();

				auto* material = Core::GetInstance()
					->GetResourceManager()
					->get<SliceEngineTypes::Material>(materialGUID)
					.get();

				if (!model || !material)
					continue;

				RCK_ModelT mdlDet = GetModelDetails(
					modelGUID.GetGUID(),
					0,
					false
				);

				uint8_t shdDet = GetShaderDetails(material->shader.get()->s);

				RCK_Size key =
					(static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset) |
					(static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset);

				BasicIDat data;
				data.mdlMtx = ptx.transform;
				data.texID = GetTextureDetails(material->albedo.get()->bindless_id);
				SetColor(data, ptx.colour);
				data.entityID = 0;

				if (ptx.colour.a > 0.999f)
					key |= MRCK_OPAQUE;
				else
					key |= MRCK_TRANSCLUCENT;

				if (key & MRCK_TRANSCLUCENT)
				{
					TranslucentCmd tc{ key, data };
					SingleExtAppend(tc.ext, material);
					translucentCmds.emplace_back(tc);
				}
				else
				{
					AppendRenderCmd(renderCmds[key], data, material);
					renderCmds[key].numVar =
						static_cast<uint32_t>(material->shader.get()->dataIn.size());
				}
			}
			
		}
		Core::GetInstance()->GetSystem<ParticleSystemManager>().particlesTransforms.clear();
	}
	void RenderCmdManager::SortTranslucent(Entity camEntity)
	{
		mLastKnownCam = camEntity;
		auto& camT = Core::GetInstance()->GetRegistry().get<Transform>(camEntity);
		glm::vec3 camFront, camRight, camUp, camPos;
		glm::mat3 camRot = glm::mat3_cast(camT.rotation);
		Core::GetInstance()->GetRenderManager()->GetCameraAxis(camRot, camFront, camRight, camUp);
		camPos = camT.GetWorldPosition() - lastTranslucentOffset;

		// Calcluate depth
		assert(sizeof(float) == 4);
		for (auto& i : translucentCmds)
		{
			glm::vec3 dir = glm::vec3(i.base.mdlMtx[3]) - camPos;
			float d = glm::dot(dir, camFront);
			i.id = (i.id & ~MRCK_DEPTH_SORT) | std::bit_cast<RCK_DepthT>(d); // clear depth first, then set val
		}

		// Sort, furthest is first
		std::sort(translucentCmds.begin(), translucentCmds.end(), [](const auto& a, const auto& b) {
			return std::bit_cast<float>(static_cast<uint32_t>(a.id & MRCK_DEPTH_SORT)) > std::bit_cast<float>(static_cast<uint32_t>(b.id & MRCK_DEPTH_SORT));
		});

		// Prefabs sorting too
		for (auto& i : prefabTranslucentCmds)
		{
			glm::vec3 dir = glm::vec3(i.base.mdlMtx[3]) - camPos;
			float d = glm::dot(dir, camFront);
			i.id = (i.id & ~MRCK_DEPTH_SORT) | std::bit_cast<RCK_DepthT>(d); // clear depth first, then set val
		}
		std::sort(prefabTranslucentCmds.begin(), prefabTranslucentCmds.end(), [](const auto& a, const auto& b) {
			return std::bit_cast<float>(static_cast<uint32_t>(a.id & MRCK_DEPTH_SORT)) > std::bit_cast<float>(static_cast<uint32_t>(b.id & MRCK_DEPTH_SORT));
			});

	}
	void RenderCmdManager::UseDrawCalls(GLuint mShader, DrawType drawType, glm::vec3 newOffset)
	{
		// Tags I need
			// Cast Shadows
			// Blend Mode: Opaque, Translucent
			// Shading Mode: Lit / unLit
			// Two-Sided?

		// Internal Sort:
			// Opaque - By Model
			// Translucent - By Dist -> then by model if possible

		// SSBO Data
			// Transform
			// Textures Samplers
			// Texture to use
			// Extra Data(?)l9

		switch (drawType)
		{
		// Only used for shadows, so dun need change shader
		case DrawType::DRAW_MODELS:
		{
			glm::vec3 offsetDelta = lastShadowOffset - newOffset;
			lastShadowOffset = newOffset;
			for (auto& i : shadowRenderCmds)
			{
				const auto& id = i.first;
				auto& batch = i.second;

				if (batch.empty())
					continue;

				ModelBasic& mdlRef = modelReferences[id];
				auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)mdlRef.mdl);
				auto& mesh = mdl.get()->meshes[mdlRef.meshOffset];
				glBindVertexArray(mesh.vao);

				//if (mdlRef.isSkin)
				{
					//for (auto& i : batch)
					//{
					//	SetModelSkinUniform(mShader, i.mdlMtx, true, i.entityID);
					//
					//}
				}
				//else
				{
					for (auto& j : batch)
						ShiftTransformMtx(j.mdlMtx, offsetDelta);

					//SetModelSkinUniform(mShader, mdlRef.isSkin, i.entityID);
					for(size_t drawCounter{}; drawCounter < batch.size(); )
					{
						size_t drawNum{ std::min(batch.size() - drawCounter, static_cast<size_t>(mMaxInstance)) };
						glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat) * drawNum, batch.data() + drawCounter);
						glDrawElementsInstanced(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr, drawNum);
						drawCounter += static_cast<GLsizei>(drawNum);
					}
				}
			}
			break;
		}
		// Used for main game
		case DrawType::DRAW_OPAQUE:
		case DrawType::DRAW_PREFAB_OPAQUE:
		{
			auto* cmds = &renderCmds;
			glm::vec3 offsetDelta{};
			if (drawType == DrawType::DRAW_PREFAB_OPAQUE)
			{
				cmds = &prefabRenderCmds;
				offsetDelta = lastRenderPrefabOffset - newOffset;
				lastRenderPrefabOffset = newOffset;
			}
			else
			{
				offsetDelta = lastRenderOffset - newOffset;
				lastRenderOffset = newOffset;
			}


			for (auto& i : *cmds)
			{
				const auto& id = i.first;
				auto& batch = i.second;

				if (batch.base.empty())
					continue;

				// Change Shader
				auto temp = static_cast<uint8_t>((id & MRCK_SHADER) >> RCK_ShaderOffset);
				auto thisShader = shaderList.at(temp);
				if (thisShader != mShader)
				{
					mShader = static_cast<GLuint>(thisShader);
					glUseProgram(mShader);
					Core::GetInstance()->GetRenderManager()->ForceSetCustomShader(std::string("CUSTOM"), mShader);
					Core::GetInstance()->GetRenderManager()->UpdateCamVP();
				}
				RCK_ModelT mdlID = static_cast<RCK_ModelT>((id & MRCK_MODEL) >> RCK_ModelOffset);
				ModelBasic& mdlRef = modelReferences[mdlID];
				auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)mdlRef.mdl);
				//auto& test = mdl.get()->meshes;
				int meshOffset = mdlRef.meshOffset;
				if (mdlRef.meshOffset >= mdl.get()->meshes.size())
					meshOffset = 0;
				auto& mesh = mdl.get()->meshes[meshOffset];
				glBindVertexArray(mesh.vao);

				for (auto& j : batch.base)
					ShiftTransformMtx(j.mdlMtx, offsetDelta);

				//GLint uniformLoc;
				if (mdlRef.isSkin)
				{
					for (size_t j{}; j < batch.base.size(); ++j)
					{
						SetModelSkinUniform(mShader, mdlRef.isSkin, batch.base[j].entityID);
						glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat), &batch.base[j]);
						glNamedBufferSubData(mEVBO, 0, sizeof(glm::uvec4), reinterpret_cast<const float*>(batch.ext.data()) + batch.numVar * j);
						glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
					}
				}
				else
				{
					SetModelSkinUniform(mShader, mdlRef.isSkin, 0);
					for (size_t drawCounter{}; drawCounter < batch.base.size(); )
					{
						size_t drawNum{ std::min(batch.base.size() - drawCounter, static_cast<size_t>(mMaxInstance)) };
						glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat) * drawNum, batch.base.data() + drawCounter);
						glNamedBufferSubData(mEVBO, 0, sizeof(float) * drawNum * batch.numVar, reinterpret_cast<const float*>(batch.ext.data()) + batch.numVar * drawCounter);
						glDrawElementsInstanced(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr, drawNum);
						drawCounter += static_cast<GLsizei>(drawNum);
					}
				}
			}
			break;
		}
		case DrawType::DRAW_TRANSLUCENT:
		case DrawType::DRAW_TRANSLUCENT_ID_ONLY:
		case DrawType::DRAW_PREFAB_TRANSLUCENT:
		case DrawType::DRAW_PREFAB_TRANSLUCENT_ID_ONLY:
		{
			RCK_ModelT currMdlID = 0xFFFF;
			glm::vec3 offsetDelta{};

			auto* cmds = &translucentCmds;
			if (drawType == DrawType::DRAW_PREFAB_TRANSLUCENT)
			{
				cmds = &prefabTranslucentCmds;
				offsetDelta = lastTranslucentPrefabOffset - newOffset;
				lastTranslucentPrefabOffset = newOffset;
			}
			else
			{
				offsetDelta = lastTranslucentOffset - newOffset;
				lastTranslucentOffset = newOffset;
			}

			for (auto& i : *cmds)
			{
				const auto& id = i.id;
				auto& dat = i.base;

				// Change Shader
				auto thisShader = static_cast<GLuint>(shaderList.at(static_cast<uint8_t>((id & MRCK_SHADER) >> RCK_ShaderOffset)));
				if (thisShader != mShader)
				{
					mShader = thisShader;
					glUseProgram(mShader);
					Core::GetInstance()->GetRenderManager()->ForceSetCustomShader(std::string("CUSTOM"), mShader);
					Core::GetInstance()->GetRenderManager()->UpdateCamVP();
					GLint uniformLoc = glGetUniformLocation(mShader, "translucentIDOnly"); 
					glUniform1i(uniformLoc, (drawType == DrawType::DRAW_TRANSLUCENT_ID_ONLY || drawType == DrawType::DRAW_PREFAB_TRANSLUCENT_ID_ONLY) ? 1 : 0);
					uniformLoc = glGetUniformLocation(mShader, "translucentSelectThreshold");
					if (uniformLoc != -1)
					{
						auto camm = Core::GetInstance()->GetRegistry().get<Camera>(mLastKnownCam);
						glUniform1f(uniformLoc, camm.translucentSelectCutoff);
					}
				}

				ShiftTransformMtx(dat.mdlMtx, offsetDelta);

				float distanceFromCam = std::bit_cast<float>(static_cast<uint32_t>(id & MRCK_DEPTH_SORT));
				if (distanceFromCam > minDistTranslucent)
				{
					RCK_ModelT mdlID = static_cast<RCK_ModelT>((id & MRCK_MODEL) >> RCK_ModelOffset);
					if (mdlID != currMdlID)
					{
						currMdlID = mdlID;
					}
					ModelBasic& mdlRef = modelReferences[currMdlID];
					auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)mdlRef.mdl);
					auto& mesh = mdl.get()->meshes[mdlRef.meshOffset];
					glBindVertexArray(mesh.vao);

					SetModelSkinUniform(mShader, mdlRef.isSkin, dat.entityID);
					glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat), &dat);
					glNamedBufferSubData(mEVBO, 0, sizeof(glm::uvec4), &i.ext);
					glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				}
			}
			break;
		}
		}
	}

	void RenderCmdManager::SingleDraw(GLuint mShader, const Entity& entity, DrawType drawType, glm::vec3 relPos)
	{
		auto core = Core::GetInstance();
		auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
		auto& rend = core->GetRegistry().get<Renderer>(entity);
		if (!rend.modelHandle.IsValid()) return;
		auto meshOffset = std::min(rend.meshOffset, static_cast<unsigned char>(rend.modelHandle.get()->meshes.size() - 1));

		auto& mesh = rend.modelHandle.get()->meshes[meshOffset];
		glBindVertexArray(mesh.vao);

		SetModelSkinUniform(mShader, (rend.skinned && !rend.modelHandle.get()->is_static), static_cast<unsigned int>(entity));
		switch (drawType)
		{
		case DrawType::DRAW_MODELS:
		{
			BasicIDat i;
			i.entityID = static_cast<unsigned int>(entity);
			i.mdlMtx = transform.transform;
			ShiftTransformMtx(i.mdlMtx, -relPos);
			glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat), &i);
			break;
		}
		case DrawType::DRAW_OPAQUE:
		{
			const SliceEngine::SliceEngineTypes::Material* material;
			if (core->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE)
			{
				if (!rend.materialInstance.albedo.IsValid() || !rend.materialInstance.shader.IsValid())
					rend.materialInstance = *(rend.materialHandle.get());
				material = &rend.materialInstance;
			}
			else
				material = rend.materialHandle.get();

			BasicIDat data{};
			data.mdlMtx = transform.transform;
			ShiftTransformMtx(data.mdlMtx, -relPos);
			SetColor(data, glm::vec4(material->color.r, material->color.g, material->color.b, 1.f));
			std::vector<glm::uvec4> ext;
			SingleExtAppend(ext, material);

			data.texID = GetTextureDetails(material->albedo.get()->bindless_id);
			data.entityID = static_cast<unsigned int>(entity);
			glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat), &data);
			glNamedBufferSubData(mEVBO, 0, sizeof(glm::uvec4), &ext);
			break;
		}
		}
		glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
	}

	void RenderCmdManager::SetModelSkinUniform(GLuint mShader, bool isSkin, unsigned int entityID)
	{
		GLint uniformLoc;
		uniformLoc = glGetUniformLocation(mShader, "skinned");
		if (isSkin && uniformLoc != -1) {
			glUniform1ui(uniformLoc, 1);

			auto core = Core::GetInstance();
			auto const& bone = core->GetRegistry().get<Bone>(static_cast<Entity>(entityID));
			Entity root_entity = bone.skeleton_root;
			if (core->GetRegistry().any_of<Animator>(root_entity)) {

				auto const& animator = core->GetRegistry().get<Animator>(root_entity);
				if (animator.Handle_curr_anim_pkg.IsValid() && animator.Handle_skeleton.IsValid())
				{
					uniformLoc = glGetUniformLocation(mShader, "final_bones_matrices");
					glUniformMatrix4fv(uniformLoc, MAX_BONES, false, glm::value_ptr(animator.GetFinalTform().data()[0]));

					glm::mat4 inverse_root = animator.inverse_map.at(bone.frame_idx);
					uniformLoc = glGetUniformLocation(mShader, "inverse_root");
					glUniformMatrix4fv(uniformLoc, 1, false, glm::value_ptr(inverse_root[0]));
				}
			}
			else {/*SLICE_LOG_ERROR("Invalid root entity for bone component when rendering");*/ }
		}
		else { glUniform1ui(uniformLoc, 0); }
	}
#pragma region Own Stored copy of data
	RenderCmdManager::RCK_ModelT RenderCmdManager::GetModelDetails(uint64_t mdlID, unsigned char meshOffset, bool isSkin)
	{
		MdlFinder mdlFinder{ mdlID, meshOffset };

		auto i = modelToIdx.find(mdlFinder);
		if (i == modelToIdx.end())
		{
			modelToIdx[mdlFinder] = static_cast<RCK_ModelT>(modelReferences.size());
			modelReferences.emplace_back(ModelBasic(mdlID, meshOffset, isSkin));
			i = modelToIdx.find(mdlFinder);
		}
		return i->second;
	}
	unsigned int RenderCmdManager::GetTextureDetails(GLuint64 bindlessID)
	{
		auto dat = textureLoaded.find(bindlessID);
		if (dat == textureLoaded.end())
		{
			unsigned int texIDX = static_cast<unsigned int>(textureList.size());
			textureLoaded.emplace(bindlessID, texIDX);
			textureList.push_back(bindlessID);
			glNamedBufferSubData(mTextureVBO, sizeof(GLuint64) * texIDX, sizeof(GLuint64), &bindlessID);
			return texIDX;
		}
		return dat->second;
	}
	uint8_t RenderCmdManager::GetShaderDetails(GLuint64 cShader)
	{
		auto dat = shaderLoaded.find(cShader);
		if (dat == shaderLoaded.end())
		{
			uint8_t shadIDX = static_cast<uint8_t>(shaderList.size());
			shaderLoaded.emplace(cShader, shadIDX);
			shaderList.push_back(cShader);
			return shadIDX;
		}
		return dat->second;
	}
	void RenderCmdManager::SingleExtAppend(std::vector<glm::uvec4>& cmd, const SliceEngineTypes::Material* mat)
	{
		if (cmd.empty())
			cmd.push_back(glm::uvec4{});
		//auto numVar = mat->shader.get()->dataIn.size();

		int mainID{}, subID{};

		for (auto i : mat->shader.get()->dataIn)
		{
			switch (i.dataType)
			{
			case SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
				cmd[mainID][subID] = static_cast<uint32_t>(std::get<bool>(mat->data.find(i.name)->second));
				break;
			case SliceEngineTypes::CustomShader::SP_TYPE::UINT:
				cmd[mainID][subID] = std::get<uint32_t>(mat->data.find(i.name)->second);
				break;
			case SliceEngineTypes::CustomShader::SP_TYPE::INT:
				cmd[mainID][subID] = static_cast<uint32_t>(std::get<int32_t>(mat->data.find(i.name)->second));
				break;
			case SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
				cmd[mainID][subID] = std::bit_cast<uint32_t>(std::get<float>(mat->data.find(i.name)->second));
				break;
			}

			if (++subID > 4)
			{
				cmd.push_back(glm::uvec4{});
				subID = 0;
				++mainID;
			}
		}
	}
	void RenderCmdManager::AppendRenderCmd(RenderCmd& rc, BasicIDat& dat, const SliceEngineTypes::Material* mat)
	{
		auto num = rc.base.size();
		rc.base.push_back(std::move(dat));

		auto numVar = mat->shader.get()->dataIn.size();

		int mainID = static_cast<int>(num * numVar / 4);
		int subID = num * numVar % 4;
		if (rc.ext.size() < mainID + 1)
			rc.ext.push_back(glm::uvec4{});
		//size_t numFloats{}, numUints{}, numInts{}, numBools{};

		for (auto i : mat->shader.get()->dataIn)
		{
			switch (i.dataType)
			{
			case SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
				rc.ext[mainID][subID] = static_cast<uint32_t>(std::get<bool>(mat->data.find(i.name)->second));
				break;
			case SliceEngineTypes::CustomShader::SP_TYPE::UINT:
				rc.ext[mainID][subID] = std::get<uint32_t>(mat->data.find(i.name)->second);
				break;
			case SliceEngineTypes::CustomShader::SP_TYPE::INT:
				rc.ext[mainID][subID] = static_cast<uint32_t>(std::get<int32_t>(mat->data.find(i.name)->second));
				break;
			case SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
				rc.ext[mainID][subID] = std::bit_cast<uint32_t>(std::get<float>(mat->data.find(i.name)->second));
				break;
			}

			if (++subID > 4)
			{
				rc.ext.push_back(glm::uvec4{});
				subID = 0;
				++mainID;
			}
		}
	}
	void RenderCmdManager::SetColor(BasicIDat& dat, const glm::vec4& color)
	{
		dat.col = static_cast<uint32_t>(color.r * 0xFF) << 24 | static_cast<uint32_t>(color.g * 0xFF) << 16 | 
				  static_cast<uint32_t>(color.b * 0xFF) << 8 | static_cast<uint32_t>(color.a * 0xFF);
	}
	void RenderCmdManager::SetAlpha(BasicIDat& dat, float alpha)
	{
		dat.col = (dat.col & 0xFFFF'FF00) | static_cast<uint32_t>(alpha * 0xFF);
	}
#pragma endregion
}
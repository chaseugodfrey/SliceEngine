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
#include "CanvasSystem.h"
#include "../Graphics/RenderManager.h" // --TODO-- Sus
#include "Systems/PrefabSystem.h"
#include "Systems/SceneSystem.h"

#include "Resource/Shader.h"
#include "Resource/Model.h"
#include "Resource/Texture.h"

// SSBOs
// Textures			  0 - Custom Shaders (Deferred.frag)
// Mat4,eID,col		  1 - Instanced.vert, Deferred.vert, debugOutline.vert, shadow.vert, pointShadow.vert (Deprecated: Deferred.frag)
// uvec4			  2 - Custom Shaders (Deferred.frag)
// Font				  3 - uiFont.vert

// UBOs
// Mat4[16]			  0 - Lighting.frag, Shadow.geom
// light data		  1 - (Deferred.frag translucent)
extern void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)


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
		CheckGLError();
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

	void RenderCmdManager::Update(float dt)
	{
		time += dt;
		while (time > maxTime)
			time -= maxTime;
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
		
		for (auto entity : view)
		{
			auto& rend = core->GetRegistry().get<Renderer>(entity);
			if (!rend.componentEnabled) continue;
			auto model = rend.modelHandle;
			if (!model.IsValid()) continue;
			const SliceEngine::SliceEngineTypes::Material* material;
			if (core->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE || core->GetSceneSystem()->mCurrentState == SceneState::PAUSE_SCENE) // --TODO-- IDK why this part also needs error check, this shouldn't happen
			{
				if (!rend.materialInstance.shader.IsValid())
					rend.materialInstance = *(rend.materialHandle.get());
				material = &rend.materialInstance;
			}
			else
				material = rend.materialHandle.get();

			auto* rcmds = &renderCmds;
			auto* rtcmds = &translucentCmds;
			bool isPrefab = core->mFactory.mRegistry.any_of<PrefabEditingEntity>(entity);
			if (isPrefab)
			{
				rcmds = &prefabRenderCmds;
				rtcmds = &prefabTranslucentCmds;
			}

			RCK_ModelT mdlDet = GetModelDetails(model.getGUID().GetGUID(), rend.meshOffset, rend.skinned && !model.get()->is_static);

			RCK_Size key = (static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset); // as long as number dun hit that high, shouldn't overload
			BasicIDat data;
			if (material->isTranslucent)
			{
				uint8_t shdDet = GetShaderDetails(material->shader.get()->translucentS);
				key = key | MRCK_TRANSCLUCENT | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
			}
			else
			{
				uint8_t shdDet = GetShaderDetails(material->shader.get()->opaqueS);
				key = key | MRCK_OPAQUE | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
			}
			SetColor(data, material->color);
			SetColor(data, material->color2, false);

			data.mdlMtx = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity).transform;
			data.entityID = static_cast<unsigned int>(entity);
			data.notLightAffected = static_cast<uint32_t>(material->isIgnoreLighting);

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
	
		// Gather Particles
		for (auto& ptx : Core::GetInstance()->GetSystem<ParticleSystemManager>().particlesTransforms)
		{
			if (!ptx.isMeshParticle)
			{
				auto model = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT);

				RCK_ModelT mdlDet = GetModelDetails(model.getGUID().GetGUID(), 0, false);
				// --MAYDO-- Currently hard set particles shader 
				SliceEngineTypes::Material tempMat;
				tempMat.shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::CustomShader>("CustomShader/particles.cshader");
				tempMat.color = ptx.colour;
				for (auto& i : tempMat.shader.get()->dataIn)
				{
					if (i.name == "texCol") // --TODO-- FR a temporary fix, plz change to material based
						tempMat.data[i.name] = ptx.textureID;
					else if (i.name == "EmissionIntensity")
						tempMat.data[i.name] = ptx.glowIntensity;
					else
						tempMat.data[i.name] = i.baseData;
				}
				RCK_Size key = (static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset); // as long as number dun hit that high, shouldn't overload

				//if (ptx.colour.a > 0.999f)
				//{
				//	uint8_t shdDet = GetShaderDetails(tempMat.shader.get()->opaqueS);
				//	key = key | MRCK_OPAQUE | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
				//}
				//else 
				{
					uint8_t shdDet = GetShaderDetails(tempMat.shader.get()->translucentS);
					key = key | MRCK_TRANSCLUCENT | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
				}

				BasicIDat data;
				data.mdlMtx = ptx.transform;
				SetColor(data, ptx.colour);
				data.entityID = 0;
				data.notLightAffected = ptx.isIgnoreLights;
				//shadowRenderCmds[mdlDet].emplace_back(ShadowInstanceData(data.mdlMtx));

				//if ((key & MRCK_TRANSLUCENCY) == MRCK_TRANSCLUCENT)
				{
					TranslucentCmd tc{ key, data };
					SingleExtAppend(tc.ext, &tempMat);
					translucentCmds.emplace_back(tc);
				}
				//else
				//{
				//	AppendRenderCmd(renderCmds[key], data, &tempMat);
				//	renderCmds[key].numVar =
				//		static_cast<uint32_t>(tempMat.shader.get()->dataIn.size());
				//}
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

				if (material->data.size() != material->shader.get()->dataIn.size())
					SliceEngine::Core::GetInstance()->GetResourceManager()->ReloadResourceInPlace(materialGUID);

				RCK_ModelT mdlDet = GetModelDetails(
					modelGUID.GetGUID(),
					0,
					false
				);

				RCK_Size key = (static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset);

				BasicIDat data;
				data.mdlMtx = ptx.transform;
				//data.texID = GetTextureDetails(material->albedo.get()->bindless_id);
				SetColor(data, ptx.colour);
				data.entityID = 0;
				data.notLightAffected = static_cast<uint32_t>(material->isIgnoreLighting);

				// --MAYDO-- Been told to turn opaque off

				//if (material->isTranslucent)
				{
					uint8_t shdDet = GetShaderDetails(material->shader.get()->translucentS);
					key |= MRCK_TRANSCLUCENT | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
				}
				//else
				//{
				//	uint8_t shdDet = GetShaderDetails(material->shader.get()->opaqueS);
				//	key |= MRCK_OPAQUE | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
				//}

				//if ((key & MRCK_TRANSLUCENCY) == MRCK_TRANSCLUCENT)
				{
					TranslucentCmd tc{ key, data };
					SingleExtAppend(tc.ext, material);
					translucentCmds.emplace_back(tc);
				}
				//else
				//{
				//	AppendRenderCmd(renderCmds[key], data, material);
				//	renderCmds[key].numVar =
				//		static_cast<uint32_t>(material->shader.get()->dataIn.size());
				//}
			}
			
		}
		Core::GetInstance()->GetSystem<ParticleSystemManager>().particlesTransforms.clear();


		auto& canvas_sys = Core::GetInstance()->GetSystem<CanvasSystem>();
		auto const& ui_entities = canvas_sys.Get_World_UI();

		auto rm = Core::GetInstance()->GetResourceManager();
		auto model = rm->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT);

		SliceEngineTypes::Material ui_mat;
		ui_mat.shader = rm->get<SliceEngineTypes::CustomShader>("CustomShader/sprite_ui.cshader");
		ui_mat.data["texCol"] = DefaultResourceIDs::COLOR_DEADED_DEFAULT;
		for (Entity ui : ui_entities) {
			auto const& tform = core->GetRegistry().get<Transform>(ui);
			if (auto sprite = core->GetRegistry().try_get<SpriteRenderer>(ui)) {
				if (sprite->componentEnabled) {

					ui_mat.color = sprite->rgba;
					ui_mat.data["texCol"] = sprite->textureHandle.GetGUID();
					ui_mat.data["U_Start"] = sprite->uv[0];
					ui_mat.data["U_End"] = sprite->uv[1];
					ui_mat.data["V_Start"] = sprite->uv[2];
					ui_mat.data["V_End"] = sprite->uv[3];

					ui_mat.isTranslucent = ui_mat.color.a < 0.999f;

					RCK_ModelT mdlDet = GetModelDetails(
						model.getGUID().GetGUID(),
						0,
						false
					);
					BasicIDat data;
					data.mdlMtx = tform.transform;
					//data.texID = GetTextureDetails(material->albedo.get()->bindless_id);
					SetColor(data, sprite->rgba);
					data.entityID = (uint32_t)ui;
					data.notLightAffected = !sprite->light_affected;

					RCK_Size key = (static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset);
					if (ui_mat.color.a > 0.999f)
					{
						uint8_t shdDet = GetShaderDetails(ui_mat.shader.get()->opaqueS);
						key |= MRCK_OPAQUE | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
					}
					else
					{
						uint8_t shdDet = GetShaderDetails(ui_mat.shader.get()->translucentS);
						key |= MRCK_TRANSCLUCENT | (static_cast<RCK_Size>(shdDet) << RCK_ShaderOffset);
					}
					if ((key & MRCK_TRANSLUCENCY) == MRCK_TRANSCLUCENT)
					{
						TranslucentCmd tc{ key, data };
						SingleExtAppend(tc.ext, &ui_mat);
						translucentCmds.emplace_back(tc);
					}
					else
					{
						AppendRenderCmd(renderCmds[key], data, &ui_mat);
						renderCmds[key].numVar =
							static_cast<uint32_t>(ui_mat.shader.get()->dataIn.size());
					}
				}
			}
			if (auto font = core->GetRegistry().try_get<FontRenderer>(ui)) {
				//ill figure this out next time
				if (font->fontHandle.GetGUID() == DefaultResourceIDs::FONT_BLANK_DEFAULT) {
					continue;
				}

				//auto const& rect = core->GetRegistry().get<RectTransform>(ui);
				//auto const& font_res = rm->get<SliceEngineTypes::Font_Data>(font->fontHandle);
				////for now dont worry about sdf, just treat it like a normal texture
				//glm::vec4 color = font->rgba;
				//float relative_scale = font->font_size / font_res->font_size;

				////fit into a line
				//struct Line {
				//	unsigned char token_count;
				//	float line_width{};
				//};
				//std::vector<Line> lines{};
				//float total_width = (float)rect.final_width;
				//float current_width = 0.f;
				//Line temp_line{};

				//for (auto const& token : font->token_list) {
				//	assert(token.char_cnt > 0);
				//	if (*token.pos == '\n') {	//if token is a line break
				//		temp_line.token_count++;
				//		temp_line.line_width = current_width;
				//		lines.push_back(temp_line);

				//		current_width = 0;
				//		temp_line.token_count = 0;
				//	}
				//	else if (current_width + token.size > total_width) {	//next token cant fit, carry over
				//		temp_line.line_width = current_width;
				//		lines.push_back(temp_line);

				//		current_width = token.size;
				//		temp_line.token_count = 1;
				//	}
				//	else {	//token can fit, append to current line
				//		temp_line.token_count++;
				//		current_width += token.size;
				//	}
				//}
				//if (temp_line.token_count) {	//any left over carried over tokens
				//	temp_line.line_width = current_width;
				//	lines.push_back(temp_line);
				//}

				//float left = -0.5f; float right = 0.5f;
				//float top = 0.5f;
			}
		}



	}
	void RenderCmdManager::SortTranslucent(Entity camEntity)
	{
		mLastKnownCam = camEntity;
		auto& camT = Core::GetInstance()->GetRegistry().get<Transform>(camEntity);
		glm::vec3 camFront, camRight, camUp, camPos;
		glm::mat3 camRot = glm::mat3_cast(camT.GetWorldRotation());
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
				int meshOffset = mdlRef.meshOffset;
				if (mdlRef.meshOffset >= mdl.get()->meshes.size())
					meshOffset = 0;
				auto& mesh = mdl.get()->meshes[meshOffset];
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
						glDrawElementsInstanced(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(drawNum));
						drawCounter += drawNum;
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
					GLint uniformLoc = glGetUniformLocation(mShader, "time");
					glUniform1f(uniformLoc, time);
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
					// Single Draws based on entity id for animations stuffs, so dun need worry abt ext data
					for (size_t i{}; i < batch.base.size(); ++i)
					{
						SetModelSkinUniform(mShader, mdlRef.isSkin, batch.base[i].entityID);
						glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat), &batch.base[i]);
						glNamedBufferSubData(mEVBO, 0, sizeof(float) * batch.numVar, reinterpret_cast<const float*>(batch.ext.data()) + batch.numVar * i);
						glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
					}
				}
				else
				{
					SetModelSkinUniform(mShader, mdlRef.isSkin, 0);
					size_t maxExtCount = static_cast<size_t>(mMaxInstance * mEVBOSafetyMult * 4 / (batch.numVar == 0 ? 1 : batch.numVar));
					for (size_t drawCounter{}; drawCounter < batch.base.size(); )
					{
						size_t drawNum{ std::min(std::min(batch.base.size() - drawCounter, static_cast<size_t>(mMaxInstance)), maxExtCount) };
						glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat) * drawNum, batch.base.data() + drawCounter);
						glNamedBufferSubData(mEVBO, 0, sizeof(float) * drawNum * batch.numVar, reinterpret_cast<const float*>(batch.ext.data()) + batch.numVar * drawCounter);
						glDrawElementsInstanced(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(drawNum));
						drawCounter += drawNum;
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

			glBindTextureUnit(2, Core::GetInstance()->GetRenderManager()->SkyboxIrradianceMap);
			glBindTextureUnit(4, Core::GetInstance()->GetRenderManager()->mDirLightDepthMaps);
			glBindTextureUnit(5, Core::GetInstance()->GetRenderManager()->mShadowCubeMapArr);

			auto godRayShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::CustomShader>("CustomShader/GodRays.cshader").get()->translucentS;
			auto* rm = Core::GetInstance()->GetRenderManager();
			glm::vec3 camPos = Core::GetInstance()->GetRegistry().get<Transform>(mLastKnownCam).GetWorldPosition();

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

				ShiftTransformMtx(dat.mdlMtx, offsetDelta);

				float distanceFromCam = std::bit_cast<float>(static_cast<uint32_t>(id & MRCK_DEPTH_SORT));
				if (distanceFromCam > minDistTranslucent)
				{

					// Change Shader
					auto thisShader = static_cast<GLuint>(shaderList.at(static_cast<uint8_t>((id & MRCK_SHADER) >> RCK_ShaderOffset)));
					if (thisShader != mShader)
					{
						mShader = thisShader;
						glUseProgram(mShader);
						rm->ForceSetCustomShader(std::string("CUSTOM"), mShader);
						rm->UpdateCamVP();
						rm->BindCameraDepth(mLastKnownCam);

						if (drawType == DrawType::DRAW_TRANSLUCENT)
						{
							if (thisShader == godRayShader)
								glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, rm->mColAttachment[rm->GOUT_GODRAY], 0);
							else
								glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, rm->mColAttachment[rm->GOUT_EMISSION], 0);
						}

						GLint uniformLoc;
						uniformLoc = glGetUniformLocation(mShader, "time");
						if (uniformLoc != -1)
							glUniform1f(uniformLoc, time);
						uniformLoc = glGetUniformLocation(mShader, "skyboxLightingPower");
						glUniform1f(uniformLoc, rm->skyboxData.lightingPower / 100.f);
						uniformLoc = glGetUniformLocation(mShader, "numLights");
						glUniform1i(uniformLoc, rm->numLightsFound);
						uniformLoc = glGetUniformLocation(mShader, "uCamPos");
						glUniform3f(uniformLoc, camPos.x, camPos.y, camPos.z);

						uniformLoc = glGetUniformLocation(mShader, "cascadeCnt");
						glUniform1i(uniformLoc, rm->mNumCascadeShadow);

						std::stringstream ss{};
						for (int i = 0; i < rm->mNumCascadeShadow; ++i)
						{
							ss.str("");
							ss << "cascadePlaneDist[" << std::to_string(i) << "]";
							uniformLoc = glGetUniformLocation(mShader, ss.str().c_str());
							if (i == rm->mNumCascadeShadow - 1)
								glUniform1f(uniformLoc, rm->mainDirLightFar);
							else
								glUniform1f(uniformLoc, rm->mainDirLightFar / rm->shadowCascadeLevels[i]);
						}
						auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(mLastKnownCam);

						uniformLoc = glGetUniformLocation(mShader, "willBloom");
						glUniform1i(uniformLoc, static_cast<GLint>(camera.postRenderToggles & RENDER_BLOOM));

						uniformLoc = glGetUniformLocation(mShader, "translucentIDOnly");
						glUniform1i(uniformLoc, (drawType == DrawType::DRAW_TRANSLUCENT_ID_ONLY || drawType == DrawType::DRAW_PREFAB_TRANSLUCENT_ID_ONLY) ? 1 : 0);
						uniformLoc = glGetUniformLocation(mShader, "translucentSelectThreshold");
						if (uniformLoc != -1)
						{
							glUniform1f(uniformLoc, camera.translucentSelectCutoff);
						}
					}

					RCK_ModelT mdlID = static_cast<RCK_ModelT>((id & MRCK_MODEL) >> RCK_ModelOffset);
					if (mdlID != currMdlID)
					{
						currMdlID = mdlID;
					}
					ModelBasic& mdlRef = modelReferences[currMdlID];
					auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)mdlRef.mdl);
					int meshOffset = mdlRef.meshOffset;
					if (mdlRef.meshOffset >= mdl.get()->meshes.size())
						meshOffset = 0;
					auto& mesh = mdl.get()->meshes[meshOffset];
					glBindVertexArray(mesh.vao);

					SetModelSkinUniform(mShader, mdlRef.isSkin, dat.entityID);
					glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat), &dat);
					glNamedBufferSubData(mEVBO, 0, sizeof(float) * i.ext.size() * 4, reinterpret_cast<const float*>(i.ext.data()));
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

		if (rend.materialHandle->data.size() != rend.materialHandle->shader.get()->dataIn.size())
			SliceEngine::Core::GetInstance()->GetResourceManager()->ReloadResourceInPlace(rend.materialHandle.getGUID());

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
			if (core->GetSceneSystem()->mCurrentState == SceneState::PLAY_SCENE || core->GetSceneSystem()->mCurrentState == SceneState::PAUSE_SCENE)
			{
				if (!rend.materialInstance.shader.IsValid()) // --TODO-- Again, this shouldn't happen
					rend.materialInstance = *(rend.materialHandle.get());
				material = &rend.materialInstance;
			}
			else
				material = rend.materialHandle.get();

			BasicIDat data{};
			data.mdlMtx = transform.transform;
			ShiftTransformMtx(data.mdlMtx, -relPos);
			SetColor(data, glm::vec4(material->color.r, material->color.g, material->color.b, 1.f));
			SetColor(data, glm::vec4(material->color2.r, material->color2.g, material->color2.b, 1.f), false);
			std::vector<glm::uvec4> ext;
			SingleExtAppend(ext, material);

			GLint uniformLoc = glGetUniformLocation(mShader, "time");
			glUniform1f(uniformLoc, time);

			data.entityID = static_cast<unsigned int>(entity);
			glNamedBufferSubData(mIVBO, 0, sizeof(BasicIDat), &data);
			glNamedBufferSubData(mEVBO, 0, sizeof(float) * material->data.size(), &ext);
			break;
		}
		}
		glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
		CheckGLError();
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
			case SliceEngineTypes::CustomShader::SP_TYPE::TEXTURE:
				auto textureGUID = (GUID)std::get<uint64_t>(mat->data.find(i.name)->second);
				auto texture = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>(textureGUID);
				if (!texture.IsValid())
					texture = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT);
				
				cmd[mainID][subID] = GetTextureDetails(texture.get()->bindless_id);
				break;
			}

			if (++subID > 3)
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

		int mainID = (num * numVar) / 4;
		int subID = (num * numVar) % 4;
		if (rc.ext.size() < mainID + 1)
			rc.ext.push_back(glm::uvec4{});

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
			case SliceEngineTypes::CustomShader::SP_TYPE::TEXTURE:
				auto textureGUID = (GUID)std::get<uint64_t>(mat->data.find(i.name)->second);
				auto texture = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>(textureGUID);
				if (!texture.IsValid())
					texture = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT);

				rc.ext[mainID][subID] = GetTextureDetails(texture.get()->bindless_id);
				break;
			}

			if (++subID > 3)
			{
				rc.ext.push_back(glm::uvec4{});
				subID = 0;
				++mainID;
			}
		}
	}
	void RenderCmdManager::SetColor(BasicIDat& dat, const glm::vec4& color, bool isFirst)
	{
		if(isFirst)
			dat.col = static_cast<uint32_t>(color.r * 0xFF) << 24 | static_cast<uint32_t>(color.g * 0xFF) << 16 | 
				  static_cast<uint32_t>(color.b * 0xFF) << 8 | static_cast<uint32_t>(color.a * 0xFF);
		else
			dat.col2 = static_cast<uint32_t>(color.r * 0xFF) << 24 | static_cast<uint32_t>(color.g * 0xFF) << 16 | 
				  static_cast<uint32_t>(color.b * 0xFF) << 8 | static_cast<uint32_t>(color.a * 0xFF);
	}
#pragma endregion
}
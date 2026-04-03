#include <pch.h>
#include "AnimationWindow.h"
#include "Selection/SelectionManager.h"
#include <Systems/SceneSystem.h>
#include <Systems/FramerateManager.h>
#include <Animator/AnimatorSystem.h>
#include <Animator/BoneSystem.h>
#include <Inspector/ComponentPropertiesGUI.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>	//just to get it working for now

namespace SliceEditor
{
	AnimationWindow::~AnimationWindow()
	{
	}

	void AnimationWindow::Init()
	{
		//AnimationPropertyGroup EventGroup;

		//EventGroup.name = "Events";
		////EventGroup.properties.push_back(AnimationProperty{ "Animation Event", std::vector<ImGui::FrameIndexType>({0, 10, 20}) });

		//mPropertyGroups.push_back(EventGroup);

		mTimeline.isPlaying = false;
		mTimeline.isLoop = false;
		mOpenEventPopup = false;
		mSequencerFlags = ImGuiNeoSequencerFlags_EnableSelection | ImGuiNeoSequencerFlags_Selection_EnableDeletion | ImGuiNeoSequencerCol_ZoomBarSlider;
	}

	bool AnimationWindow::CheckForAnimator()
	{
		// Check if any entities selected
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		if (!(selectionManager->mSelectionType == SelectionType::ENTITY || selectionManager->mSelectionType == SelectionType::PREFAB_ENTITY))
		{
			ClearData();
			return false;
		}
		auto& nodes = selectionManager->GetSelectedNodes();
		Entity entity = entt::null;

		// if entities present
		if (nodes.size() > 0)
		{
			EntityNode* entityNode = static_cast<EntityNode*>(*nodes.begin());
			entity = entityNode->entity;

			// check if first entity has animator component
			auto anim = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Animator>(entity);
			//tmpEnt = entity;
			// if anim exists
			

			// the valid will fail cos if we add a animator component to something for non bone animation it will nvr hit the requirement of having valid skeleton
			if (anim /*&& anim->IsValid()*/)
			{
				// if current animator is null or mismatch
				// ignore if anim == mCurrentAnimator
				// either case, return true
				if (!mCurrentAnimator || anim != mCurrentAnimator)
				{	
					
					LoadDataFromAnimator(anim, entity);
					//mCurrentTransform = &SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entity);
					return true;
				}

				if (tmpEnt != entity)
				{
					LoadDataFromAnimator(anim, entity);
					//mCurrentTransform = &SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entity);
					tmpEnt = entity;
					return true;
				}

				return true;
			}

			// if retrieved ptr is null, unload animator and destroy data
			else
			{
				ClearData();
				return false;
			}
		}

		// if no entities present
		else
		{
			ClearData();
			return false;
		}

		return false;
	}

	void AnimationWindow::LoadDataFromAnimator(SliceEngine::Animator* component, entt::entity entity)
	{
		mCurrentAnimator = component;

		animationClips.clear();
		customAnimClips.clear();

		if ((mCurrentAnimator->curr_anim_pkg.animations.size() == 0 && mCurrentAnimator->Handle_skeleton.IsValid()))
		{
			return;
		}

		if (mCurrentAnimator->curr_anims.animations.size() == 0 && !mCurrentAnimator->Handle_skeleton.IsValid())
		{
			return;
		}

		mCurrentClipIndex = 0;
		mPropertyGroups.clear();

		if(mCurrentAnimator->Handle_skeleton.IsValid())
		{
			animationClips.reserve(mCurrentAnimator->curr_anim_pkg.animations.size());

			for (auto& anim : mCurrentAnimator->curr_anim_pkg.animations)
			{
				animationClips.push_back(&anim);
			}

			LoadDataFromAnimationClip(*animationClips[0], mCurrentClipIndex);
		}
		else
		{
			customAnimClips.reserve(mCurrentAnimator->curr_anims.animations.size());

			for (auto& anim : mCurrentAnimator->curr_anims.animations)
			{
				customAnimClips.push_back(anim);
			}

			LoadDataFromSequenceClip(customAnimClips[0], mCurrentClipIndex);
		}

		// add 0 check for size()

		if(!mCurrentAnimator->Handle_skeleton.IsValid())
		{
			//std::string name = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetName();
			//AnimationPropertyGroup transformGroup;

			//transformGroup.name = name + " Transform";
			//transformGroup.properties.push_back(AnimationProperty{ "Position.x", std::vector<ImGui::FrameIndexType>({0, 10, 20}) });
			//transformGroup.properties.push_back(AnimationProperty{ "Position.y", std::vector<ImGui::FrameIndexType>({0, 10, 20}) });
			//transformGroup.properties.push_back(AnimationProperty{ "Position.z", std::vector<ImGui::FrameIndexType>({0, 10, 20}) });

			//mPropertyGroups.push_back(transformGroup);

			auto& engine_reg = SliceEngine::Core::GetInstance()->GetRegistry();
			auto& scene_graph = engine_reg.get<SliceEngine::SceneGraph>(entity);

			LoadPropertyGroup(entity, scene_graph);
		}
	}

	void AnimationWindow::LoadDataFromAnimationClip(SliceEngine::SliceEngineTypes::Animation& animClip, size_t animClipIdx)
	{
		endFrame = animClip.duration * animClip.fps;
		startFrame = 0;
		currentFrame = 0;
		mCurrentTime = 0;

		if (!mPropertyGroups.empty())
		{
			if (std::strcmp(mPropertyGroups[0].name.c_str(), "Events") == 0)
			{
				mPropertyGroups.erase(mPropertyGroups.begin());
			}
		}

		AnimationPropertyGroup EventGroup;

		EventGroup.name = "Events";
		std::vector<ImGui::FrameIndexType> eventFrames{};

		for (int i = 0; i < mCurrentAnimator->eventFrames.size(); i++)
		{
			if (mCurrentAnimator->eventFrames[i].animIdx == static_cast<unsigned int>(animClipIdx))
			{
				eventFrames.push_back(mCurrentAnimator->eventFrames[i].frameNumber);
			}

		}

		EventGroup.properties.push_back(AnimationProperty{ "Animation Event", eventFrames });

		mPropertyGroups.insert(mPropertyGroups.begin(),EventGroup);
	}

	void AnimationWindow::LoadDataFromSequenceClip(SliceEngine::SliceEngineTypes::Sequence& animClip, size_t animClipIdx)
	{
		endFrame = animClip.duration * animClip.fps;
		startFrame = 0;
		currentFrame = 0;
		mCurrentTime = 0;

		if (!mPropertyGroups.empty())
		{
			mPropertyGroups.erase(mPropertyGroups.begin(),mPropertyGroups.end());
		}

		AnimationPropertyGroup EventGroup;

		EventGroup.name = "Events";
		std::vector<ImGui::FrameIndexType> eventFrames{};

		for (int i = 0; i < mCurrentAnimator->eventFrames.size(); i++)
		{
			if (mCurrentAnimator->eventFrames[i].animIdx == static_cast<unsigned int>(animClipIdx))
			{
				eventFrames.push_back(mCurrentAnimator->eventFrames[i].frameNumber);
			}

		}

		EventGroup.properties.push_back(AnimationProperty{ "Animation Event", eventFrames });

		mPropertyGroups.insert(mPropertyGroups.begin(), EventGroup);

		std::string name = SliceEngine::FactoryInstance.GetGOByEntity(tmpEnt).GetName();
		AnimationPropertyGroup transformGroup;

		transformGroup.name = name + " Transform";
		std::vector <ImGui::FrameIndexType> trfFrames{};

		if (mCurrentAnimator->curr_anims.animations.size() > 0)
		{
			for (const auto& [frame, trfs] : mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform)
			{
				trfFrames.push_back(static_cast<int32_t>(frame));
			}
		}

		transformGroup.properties.push_back(AnimationProperty{ "Position.x", trfFrames });
		transformGroup.properties.push_back(AnimationProperty{ "Position.y", trfFrames });
		transformGroup.properties.push_back(AnimationProperty{ "Position.z", trfFrames });

		mPropertyGroups.push_back(transformGroup);

		AnimationPropertyGroup rotationGroup;

		rotationGroup.name = name + " Rotation";
		std::vector <ImGui::FrameIndexType> rotFrames{};

		if (mCurrentAnimator->curr_anims.animations.size() > 0)
		{
			for (const auto& [frame, rot] : mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation)
			{
				rotFrames.push_back(static_cast<int32_t>(frame));
			}
		}

		rotationGroup.properties.push_back(AnimationProperty{ "Rotation.x", rotFrames });
		rotationGroup.properties.push_back(AnimationProperty{ "Rotation.y", rotFrames });
		rotationGroup.properties.push_back(AnimationProperty{ "Rotation.z", rotFrames });

		mPropertyGroups.push_back(rotationGroup);

		AnimationPropertyGroup scaleGroup;

		scaleGroup.name = name + " Scale";
		std::vector <ImGui::FrameIndexType> scaleFrames{};

		if (mCurrentAnimator->curr_anims.animations.size() > 0)
		{
			for (const auto& [frame, scale] : mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale)
			{
				scaleFrames.push_back(static_cast<int32_t>(frame));
			}
		}

		scaleGroup.properties.push_back(AnimationProperty{ "Scale.x", scaleFrames });
		scaleGroup.properties.push_back(AnimationProperty{ "Scale.y", scaleFrames });
		scaleGroup.properties.push_back(AnimationProperty{ "Scale.z", scaleFrames });

		mPropertyGroups.push_back(scaleGroup);
	}

	void AnimationWindow::LoadPropertyGroup(entt::entity entity, SliceEngine::SceneGraph& scene_graph)
	{
		auto& engine_reg = SliceEngine::Core::GetInstance()->GetRegistry();
		auto child_entity = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

		while (child_entity != entt::null)
		{
			auto& child_scene_graph = engine_reg.get<SliceEngine::SceneGraph>(child_entity);
			auto trf = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Transform>(child_entity);

			if (trf)
			{
				std::string name = SliceEngine::FactoryInstance.GetGOByEntity(child_entity).GetName();
				AnimationPropertyGroup transformGroup;

				transformGroup.name = name + " Transform";

				std::vector <ImGui::FrameIndexType> trfFrames{};

				if (mCurrentAnimator->curr_anims.animations.size() > 0)
				{
					for(const auto& [frame,trfs] : mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform)
					{
						trfFrames.push_back(static_cast<int32_t>(frame));
					}
				}

				transformGroup.properties.push_back(AnimationProperty{ "Position.x", trfFrames });
				transformGroup.properties.push_back(AnimationProperty{ "Position.y", trfFrames });
				transformGroup.properties.push_back(AnimationProperty{ "Position.z", trfFrames });

				mPropertyGroups.push_back(transformGroup);
			}

			LoadPropertyGroup(child_entity, child_scene_graph);

			child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
		}
	}

	void AnimationWindow::ClearData()
	{
		//if (!mCurrentAnimator)
			//return;

		mCurrentAnimator = nullptr;
		tmpEnt = entt::null;

		mPropertyGroups.clear();
	}

	void AnimationWindow::UpdateTransform(SliceEngine::SliceEngineTypes::Animation* animClip, float time)
	{
		animClip->UpdateTransforms(mCurrentAnimator->final_tforms, time, *mCurrentAnimator->Handle_skeleton.get());
		UpdateBoneScene(tmpEnt);
		UpdateBones();
	}

	void AnimationWindow::UpdateBoneScene(Entity ent)
	{
		auto core = SliceEngine::Core::GetInstance();

		auto const& cAnimator = core->GetRegistry().try_get<SliceEngine::Animator>(ent);

		if (cAnimator)
		{
			if (auto scene_graph = core->GetRegistry().try_get<SliceEngine::SceneGraph>(ent)) {
				entt::entity child = scene_graph->neighbours[SliceEngine::SceneGraph::DOWN];
				while (child != entt::null)
				{
					UpdateBoneScene(child);
					child = core->GetRegistry().get<SliceEngine::SceneGraph>(child).neighbours[SliceEngine::SceneGraph::RIGHT];
				}
			}
		}
		else
		{
			auto const& bone = core->GetRegistry().try_get<SliceEngine::Bone>(ent);
			if (bone)
			{
				Entity root_entity = bone->skeleton_root;
				if (root_entity != ent)
				{
					//auto& animator = core->GetRegistry().get<SliceEngine::Animator>(root_entity);
					auto& transform = core->GetRegistry().get<SliceEngine::Transform>(ent);

					//if (!mTimeline.isPlaying)
						///continue;

					//some pseudo code
					glm::mat4 const& frame = mCurrentAnimator->GetFinalTform()[bone->frame_idx];
					glm::vec3 translation, scale, skew;
					glm::vec4 perspective;
					glm::quat rotation;
					glm::decompose(frame, scale, rotation, translation, skew, perspective);
					transform.position = translation;
					transform.rotation = rotation;
					transform.scale = scale;

					//if is a renderer, tell skeleton to calculate inverse for this index
					if (core->GetRegistry().any_of<SliceEngine::Renderer>(ent)) {
						mCurrentAnimator->inverse_flags.set(bone->frame_idx);
					}
				}

				if (auto scene_graph = core->GetRegistry().try_get<SliceEngine::SceneGraph>(ent)) {
					entt::entity child = scene_graph->neighbours[SliceEngine::SceneGraph::DOWN];
					while (child != entt::null)
					{
						UpdateBoneScene(child);
						child = core->GetRegistry().get<SliceEngine::SceneGraph>(child).neighbours[SliceEngine::SceneGraph::RIGHT];
					}
				}
			}
		}
	}

	void AnimationWindow::UpdateBones()
	{
		//SliceEngine::Animator& animator = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Animator>(entity);
		SliceEngine::Transform& transform = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(tmpEnt);
		if (mCurrentAnimator->is_bone)
		{
			auto& anim = animationClips[mCurrentClipIndex];

			anim->ApplyParentTransforms(mCurrentAnimator->final_tforms, *mCurrentAnimator->Handle_skeleton.get(), transform.transform);
			mCurrentAnimator->SetInverseRoots();
			anim->ApplyInverseBind(mCurrentAnimator->final_tforms, *mCurrentAnimator->Handle_skeleton.get());
		}
	}

	void AnimationWindow::Draw()
	{
		ImGui::Begin("Animation");
		bool hasAnimator = CheckForAnimator();
		bool isSkeleton{}; 
		size_t clipSize{};

		//ImVec2 windowSize = ImGui::GetContentRegionAvail();

		

		if (hasAnimator)
		{
			isSkeleton = mCurrentAnimator->Handle_skeleton.IsValid();

			if (isSkeleton)
				clipSize = animationClips.size();
			else
				clipSize = customAnimClips.size();
		}


		ImVec2 p0 = ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMin();
		ImVec2 p1 = ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMax();
		ImGuiID id = ImGui::GetCurrentWindow()->GetID("AnimationWindowPassive");
		ImRect rect(p0, p1);

		if (ImGui::BeginDragDropTargetCustom(rect, id))
		{
			if (ImGui::AcceptDragDropPayload("SequencePackage"))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SequencePackage"))
				{
					SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
					mCurrentAnimator->Handle_Anims = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::SequencePackage>(recievedPayload);
					if (mCurrentAnimator->Handle_Anims.IsValid())
					{
						mCurrentAnimator->curr_anims = *mCurrentAnimator->Handle_Anims.get();
						mCurrentAnimator->stateMachine.InitState(mCurrentAnimator->curr_anims);
						auto anim = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Animator>(tmpEnt);
						if (anim)
							LoadDataFromAnimator(anim, tmpEnt);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		// disable if no selection
		if (!hasAnimator || clipSize == 0)
			ImGui::BeginDisabled();

#pragma region Animation Toolbar

		ImGui::BeginGroup();

		bool ret = false;

		// Controls

		std::string playOrPause = mTimeline.isPlaying ? "Pause" : "Play";

		if (ImGui::Button(playOrPause.c_str()))
		{
			mTimeline.isPlaying = !mTimeline.isPlaying;
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop"))
		{
			//auto core = SliceEngine::Core::GetInstance();

			mTimeline.isPlaying = false;
			if(clipSize > 0)
			{
				if(isSkeleton)
				{
					LoadDataFromAnimationClip(*animationClips[mCurrentClipIndex], mCurrentClipIndex);

					UpdateTransform(animationClips[mCurrentClipIndex], 0);
				}

				else
				{
					LoadDataFromSequenceClip(customAnimClips[mCurrentClipIndex], mCurrentClipIndex);

					// update trf
				}
			}
			//UpdateBoneScene(tmpEnt);
			//UpdateBones();
		}

		ImGui::SameLine();

		bool wasLoop = mTimeline.isLoop;
		if (wasLoop)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

		if (ImGui::Button("Loop"))
		{
			mTimeline.isLoop = !mTimeline.isLoop;
		}

		if (wasLoop)
			ImGui::PopStyleColor();

		ImGui::SameLine();

		if (ImGui::Button("Add Event"))
		{
			//auto core = SliceEngine::Core::GetInstance();
			// pop up type script name and script func
			std::string scriptName{};
			std::string scriptFunc{};

			//Add the event to the eventFrames vector
			
			bool frameHasEvent = false;
			for (auto& event : mCurrentAnimator->eventFrames)
			{
				if (event.animIdx == mCurrentClipIndex && event.frameNumber == static_cast<unsigned int>(currentFrame))
				{
					SLICE_LOG_WARNING("Trying to Create an Event on a frame that already has an event!");
					frameHasEvent = true;
				}
			}
			if(!frameHasEvent)
			{
				if(clipSize > 0)
				{
					mCurrentAnimator->eventFrames.push_back(SliceEngine::SliceEngineTypes::AnimationKeyFrame{ scriptName,scriptFunc,static_cast<unsigned int>(mCurrentClipIndex),static_cast<unsigned int>(currentFrame)});
					
					if(isSkeleton)
					{
						LoadDataFromAnimationClip(mCurrentAnimator->Handle_curr_anim_pkg.get()->animations[mCurrentClipIndex], mCurrentClipIndex);
					}
					else
					{
						LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);

					}
				}
			}
		}

		
		
		// check for non bone anim stuff
		if (!isSkeleton && hasAnimator)
		{
			if (!mCurrentAnimator->Handle_Anims.IsValid())
			{
				ImGui::EndDisabled();
				ImGui::SameLine();

				//static std::filesystem::path targetAnimsPath = std::filesystem::current_path();
				static std::filesystem::path targetAnimsPath = std::filesystem::path("Assets");


				if (ImGui::Button("Create Animation Package"))
				{
					if (targetAnimsPath.filename() != "Animations")
					{
						targetAnimsPath = targetAnimsPath / "Animations";
						//std::filesystem::current_path(target);

						ImGui::OpenPopup("##SaveAnims_Popup");
					}
				}

				if (ImGui::BeginPopupModal("##SaveAnims_Popup", nullptr))
				{
					static std::string newAnimsName = "";

					// so what changed? why no update string name


					if (StringInputHeader(mRegistry, "New File Name: ", "##newFileAnims", newAnimsName))
					{

					}

					if (ImGui::Button("Save Changes"))
					{
						if(!newAnimsName.empty())
						{
							targetAnimsPath = targetAnimsPath / newAnimsName;
							if (targetAnimsPath.extension() != ".seqpkg")
							{
								targetAnimsPath += ".seqpkg";
							}

							// like this to save new resource?

							std::string relativeAnimsPath = "Animations/" + newAnimsName + ".seqpkg";

							SequencePkgData Anims{};
							Anims.SerializeAsset(targetAnimsPath);

							mRegistry.GetAssetManager().CreateResource(targetAnimsPath, nullptr, true);
							mCurrentAnimator->Handle_Anims = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::SequencePackage>(mRegistry.GetAssetManager().mFilenameToGUID[relativeAnimsPath]);
							UnLoadSequencePkgData(Anims, mCurrentAnimator->curr_anims);

							newAnimsName = "";
							targetAnimsPath = std::filesystem::path("Assets");
							ImGui::CloseCurrentPopup();

							if(mCurrentAnimator->curr_anims.animations.size() > 0)
								LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
						}
					}

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				ImGui::BeginDisabled();
			}

			else
			{
				if(clipSize == 0)
					ImGui::EndDisabled();

				ImGui::SameLine();

				
				//static std::filesystem::path targetAnimPath = std::filesystem::current_path();
				static std::filesystem::path targetAnimPath = std::filesystem::path("Assets");
				//std::filesystem::path filePath = mAssetDirectory.string() + "/" + "Prefabs" + "/" + mRegistry.get<SliceEntity>(tmpEnt).mName + ".prefab";

				if (ImGui::Button("Add Animation"))
				{
					if (targetAnimPath.filename() != "Animations")
					{
						targetAnimPath = targetAnimPath / "Animations";
						//std::filesystem::current_path(target);

					}
					ImGui::OpenPopup("SaveAnim_Popup");
				}

				if (ImGui::BeginPopupModal("SaveAnim_Popup", nullptr))
				{
					static std::string newAnimName = "";
					if (StringInputHeader(mRegistry, "New File Name: ", "##newFileAnim", newAnimName))
					{

					}

					if (ImGui::Button("Save Changes"))
					{
						if(!newAnimName.empty())
						{
							targetAnimPath = targetAnimPath / newAnimName;
							if (targetAnimPath.extension() != ".seq")
							{
								targetAnimPath += ".seq";
							}

							SliceEngine::SliceEngineTypes::Sequence newAnim{};
							newAnim.name = newAnimName;

							std::string relativeAnimPath = "Animations/" + newAnimName + ".seq";

							SequenceData animData{};
							animData.LoadSequenceData(newAnim);
							animData.SerializeAsset(targetAnimPath);

							mRegistry.GetAssetManager().CreateResource(targetAnimPath, nullptr, true);
							//mCurrentAnimator->Handle_Anims = mRegistry.GetAssetManager().mFilenameToGUID[relativeAnimPath];

							mCurrentAnimator->curr_anims.animations.push_back(newAnim);
							customAnimClips.push_back(newAnim);

							std::optional<std::string> parentName = mRegistry.GetAssetManager().GetFilenameFromGUID(mCurrentAnimator->Handle_Anims.getGUID());
							if (parentName)
							{
								std::filesystem::path parentPath = std::filesystem::current_path() / parentName.value();
								SequencePkgData parentPkg{};
								parentPkg.DeserializeAsset(parentPath);
								parentPkg.animations.push_back(newAnim.name);
								parentPkg.SerializeAsset(parentPath);
							}

							newAnimName = "";
							targetAnimPath = std::filesystem::path("Assets");;
							ImGui::CloseCurrentPopup();

							LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
						}
					}

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Save"))
				{
					auto animsFilePath = mRegistry.GetAssetManager().GetFilenameFromGUID(mCurrentAnimator->Handle_Anims.getGUID());
					if (animsFilePath.has_value())
					{
						SequencePkgData Anims{};
						Anims.LoadSequencePkgData(mCurrentAnimator->curr_anims);

						Anims.SerializeAsset(mRegistry.GetAssetManager().mAssetDirectory / animsFilePath.value());
					}

					// also add for animations
					for (auto& anim : mCurrentAnimator->curr_anims.animations)
					{
						std::string animFilePath = "Animations/" + anim.name + ".seq";

						SequenceData animData{};
						animData.LoadSequenceData(anim);

						animData.SerializeAsset(mRegistry.GetAssetManager().mAssetDirectory / animFilePath);
					}
				}

				if (clipSize == 0)
					ImGui::BeginDisabled();

				//ImGui::SameLine();
				if (mCurrentAnimator->curr_anims.animations.size() > 0)
				{
					float durationBuffer = mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].duration;

					DragFloatInputHeader(mRegistry, "Duration:", "##anim_duration", durationBuffer);

					if (std::abs(durationBuffer - mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].duration) > FLT_EPSILON)
					{
						mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].duration = durationBuffer;
						customAnimClips[mCurrentClipIndex].duration = durationBuffer;
						LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
					}

					//ImGui::SameLine();

					UINT32 frameBuffer = mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].fps;

					DragUInt32InputHeader(mRegistry, "FPS:", "##anim_frames", frameBuffer, "%u", 0U, 240U);

					if (std::abs(static_cast<int>(frameBuffer) - static_cast<int>(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].fps)) > 0)
					{
						mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].fps = frameBuffer;
						customAnimClips[mCurrentClipIndex].fps = frameBuffer;
						LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
					}
				}
			}
		}
		
		// run timeline here temporarily

		ImGui::EndGroup();
#pragma endregion

		ImGui::Separator();

#pragma region Animation Timeline
		ImGui::BeginGroup();

		std::string preview = "No Animations";
		std::vector<std::string> animationClipNames;
		std::string animationName;

		if (hasAnimator)
		{
			if (clipSize > 0)
			{
				if(isSkeleton)
				{
					size_t pos_ = animationClips[0]->name.find_first_of('|');
					if (pos_ != std::string::npos)
					{
						animationName = animationClips[0]->name.substr(0, pos_) + " Animation: ";
					}
					for (auto animationClip : animationClips)
					{
						size_t pos = animationClip->name.find_first_of('|');
						std::string clipName;

						if (pos != std::string::npos)
						{
							clipName = animationClip->name.substr(pos + 1);
						}
						else
						{
							clipName = animationClip->name;
						}

						animationClipNames.push_back(clipName);
					}
				}
				else
				{
					size_t pos_ = customAnimClips[0].name.find_first_of('|');
					if (pos_ != std::string::npos)
					{
						animationName = customAnimClips[0].name.substr(0, pos_) + " Animation: ";
					}
					for (auto animationClip : customAnimClips)
					{
						size_t pos = animationClip.name.find_first_of('|');
						std::string clipName;

						if (pos != std::string::npos)
						{
							clipName = animationClip.name.substr(pos + 1);
						}
						else
						{
							clipName = animationClip.name;
						}

						animationClipNames.push_back(clipName);
					}
				}
			}
			else
			{
				animationClipNames.push_back("No Animations");
			}
		}
		else
		{
			animationClipNames.push_back("No Animations");
		}

		if (animationClipNames.size() == 1 && std::strcmp(animationClipNames[0].c_str(), "No Animations") == 0)
		{
			mCurrentClipIndex = 0;
		}

		if (ComboHeader(mRegistry, animationName.c_str(), "##animSelected", mCurrentClipIndex, animationClipNames, true))
		{
			if (isSkeleton)
				LoadDataFromAnimationClip(mCurrentAnimator->Handle_curr_anim_pkg.get()->animations[mCurrentClipIndex], mCurrentClipIndex);
			else
				LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
		}

		ImGui::EndGroup();

		// fixing the last element cut off byt setting the height,
		float lineLayerHeight = 30.0f;
		float totalHeight = mPropertyGroups.size() * 20.0f;
		for (auto& group : mPropertyGroups) 
		{
			totalHeight += group.properties.size() * lineLayerHeight;
		}
		totalHeight += 50.0f;

		if (ImGui::BeginNeoSequencer("Animation Sequencer", &currentFrame, &startFrame, &endFrame, { 0,totalHeight }, mSequencerFlags))
		{			
			for (auto& group : mPropertyGroups)
			{
				if (ImGui::BeginNeoGroup(group.name.c_str(), &group.isOpen))
				{
					for (auto& property : group.properties)
					{
						if (ImGui::BeginNeoTimelineEx(property.name.c_str(), &group.isOpen))
						{
							if (ImGui::IsItemHovered())
							{
								// if its a transform row
								if (property.name.find("Position") != std::string::npos)
								{
									if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
									{
										mOpenTrfOption = true;
										mOpenSRTVar = 0;
									}
								}

								if (property.name.find("Rotation") != std::string::npos)
								{
									if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
									{
										mOpenTrfOption = true;
										mOpenSRTVar = 1;
									}
								}
								if (property.name.find("Scale") != std::string::npos)
								{
									if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
									{
										mOpenTrfOption = true;
										mOpenSRTVar = 2;
									}
								}
							}


							for (auto& key : property.keys)
							{
								ImGui::NeoKeyframe(&key);

								if (ImGui::IsNeoKeyframeHovered())
								{
									if (ImGui::IsNeoKeyframeRightClicked())
									{
										// only for animation events nibba
										if (std::strcmp(property.name.c_str(), "Animation Event") == 0)
										{
											mOpenEventOption = true;
											mCurrentKeyIndex = key;
											//ImGui::OpenPopup("Keyframe Context");
											//Set the mCurrentEventIndex for the pop-up
											auto it = std::find_if(mCurrentAnimator->eventFrames.begin(), mCurrentAnimator->eventFrames.end(), [&](const SliceEngine::SliceEngineTypes::AnimationKeyFrame& x)
												{
													return (x.frameNumber == static_cast<unsigned int>(key) && x.animIdx == static_cast<unsigned int>(mCurrentClipIndex));
												});

											if (it != mCurrentAnimator->eventFrames.end())
											{
												mCurrentEventIndex = static_cast<int>(std::distance(mCurrentAnimator->eventFrames.begin(), it));
											}
										}

										if (property.name.find("Position") != std::string::npos)
										{
											mOpenTrfEdit = true;
											mCurrentKeyIndex = key;
											mOpenSRTVarEdit = 0;

											auto it = std::find_if(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.begin(), mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.end(), [&](const std::pair<unsigned int, glm::vec3>& x)
												{
													return (x.first == static_cast<unsigned int>(key));
												});

											if (it != mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.end())
											{
												mCurrentEventIndex = static_cast<int>(std::distance(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.begin(), it));
											}
										}
										if (property.name.find("Rotation") != std::string::npos)
										{
											mOpenTrfEdit = true;
											mCurrentKeyIndex = key;
											mOpenSRTVarEdit = 1;

											auto it = std::find_if(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation.begin(), mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation.end(), [&](const std::pair<unsigned int, glm::vec3>& x)
												{
													return (x.first == static_cast<unsigned int>(key));
												});

											if (it != mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation.end())
											{
												mCurrentEventIndex = static_cast<int>(std::distance(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation.begin(), it));
											}
										}
										if (property.name.find("Scale") != std::string::npos)
										{
											mOpenTrfEdit = true;
											mCurrentKeyIndex = key;
											mOpenSRTVarEdit = 2;

											auto it = std::find_if(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale.begin(), mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale.end(), [&](const std::pair<unsigned int, glm::vec3>& x)
												{
													return (x.first == static_cast<unsigned int>(key));
												});

											if (it != mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale.end())
											{
												mCurrentEventIndex = static_cast<int>(std::distance(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale.begin(), it));
											}
										}
									}
								}
							}
							ImGui::EndNeoTimeLine();
						}
					}
					ImGui::EndNeoGroup();
				}
			}
			ImGui::EndNeoSequencer();
		}

		// click on neosequencer
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) 
		{
			ImGui::SetWindowFocus();
		}

		//Keyframe Context?
		if(mOpenEventOption)
		{
			if (ImGui::BeginPopupContextItem("Keyframe Context"))
			{

				if (ImGui::Selectable("Edit Event"))
				{
					mOpenEventPopup = true;
					mOpenEventOption = false;
				}

				if (ImGui::Selectable("Delete Event"))
				{
					mCurrentAnimator->eventFrames.erase(mCurrentAnimator->eventFrames.begin() + mCurrentEventIndex);
					std::erase(mPropertyGroups[0].properties[0].keys, mCurrentKeyIndex);
					//mPropertyGroups[0].properties[0].keys.erase(mPropertyGroups[0].properties[0].keys.begin() + mCurrentKeyIndex);
					mOpenEventOption = false;
				}

				ImGui::EndPopup();
			}
		}

		if (mOpenTrfEdit)
		{
			AnimatorSRTPopupEdit();
		}

		if (mOpenTrfEditKeyAttrib)
		{
			AnimatorSRTPopupEditKeyAttrib();
		}

		if (mOpenTrfOption)
		{
			AnimatorSRTPopup();
		}

		// only update when on window
		if(ImGui::IsWindowFocused())
		{
			if(clipSize > 0)
			{
				auto core = SliceEngine::Core::GetInstance();
				if (mCurrentAnimator && hasAnimator && core->GetSceneSystem()->mCurrentState == SliceEngine::DEFAULT)
				{
					if (mTimeline.isPlaying)
					{
						if(isSkeleton)
							currentFrame = static_cast<ImGui::FrameIndexType>(mCurrentTime * animationClips[mCurrentClipIndex]->fps);
						else
							currentFrame = static_cast<ImGui::FrameIndexType>(mCurrentTime * customAnimClips[mCurrentClipIndex].fps);


						if (currentFrame > endFrame)
						{
							currentFrame = startFrame;
						}
						for (size_t step = 0; step < core->GetFramerateManager()->getCurrentNumberOfSteps(); ++step)
						{
							float dt = static_cast<float>(core->GetFramerateManager()->getFixedDeltaTime());
							if(!isSkeleton)
								mCurrentTime += dt * mCurrentAnimator->stateMachine.EFSM.stateMap[mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].name].animationSpeed;
							else
								mCurrentTime += dt * mCurrentAnimator->stateMachine.EFSM.stateMap[mCurrentAnimator->curr_anim_pkg.animations[mCurrentClipIndex].name].animationSpeed;
						}
					}

					else
					{
						if(isSkeleton)
							mCurrentTime = static_cast<float>(currentFrame) / static_cast<float>(animationClips[mCurrentClipIndex]->fps);
						else
							mCurrentTime = static_cast<float>(currentFrame) / static_cast<float>(customAnimClips[mCurrentClipIndex].fps);
					}


					//Bone animation
					if (isSkeleton)
					{
						auto& anim = animationClips[mCurrentClipIndex];
						if (anim->duration <= 0.0f)
						{
							mCurrentTime = 0.0f;
						}
						else
						{
							if (mCurrentTime > anim->duration)
							{

								if (!mTimeline.isLoop)
								{
									mTimeline.isPlaying = false;
									currentFrame = startFrame;
									mCurrentTime = 0.0f;
									ret = true;
								}
								else
								{
									mTimeline.isPlaying = true;
									mCurrentTime = std::fmod(mCurrentTime, anim->duration);

								}
							}
						}

						if (!ret)
						{
							float safe_time = std::min(mCurrentTime, anim->duration);
							//anim->UpdateTransforms(mCurrentAnimator->final_tforms, safe_time, *mCurrentAnimator->Handle_skeleton.get());
							UpdateTransform(anim, safe_time);
							//UpdateBoneScene(tmpEnt);
							//UpdateBones();
						}
					}

					else
					{
						auto& anim = customAnimClips[mCurrentClipIndex];
						if (anim.duration <= 0.0f)
						{
							mCurrentTime = 0.0f;
						}
						else
						{
							if (mCurrentTime > anim.duration)
							{

								if (!mTimeline.isLoop)
								{
									mTimeline.isPlaying = false;
									currentFrame = startFrame;
									mCurrentTime = 0.0f;
									ret = true;
								}
								else
								{
									mTimeline.isPlaying = true;
									mCurrentTime = std::fmod(mCurrentTime, anim.duration);

								}
							}
						}

						if (!ret)
						{
							float safe_time = std::min(mCurrentTime, anim.duration);
							anim.UpdateTransforms(SliceEngine::Core::GetInstance()->GetRegistry(), tmpEnt, safe_time);
							//UpdateTransform(anim, safe_time);
						}
					}
				}
			}
		}

		if (mOpenEventPopup)
		{
			ImGui::OpenPopup("AnimationEventPopup");
			if (isSkeleton)
				AnimatorEventPopup(mCurrentAnimator->Handle_curr_anim_pkg.get()->animations[mCurrentClipIndex], mCurrentClipIndex, mCurrentAnimator->eventFrames[mCurrentEventIndex]);
			else
				AnimatorEventPopupCustom(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex, mCurrentAnimator->eventFrames[mCurrentEventIndex]);
		}

#pragma endregion

		if (!hasAnimator || clipSize == 0)
			ImGui::EndDisabled();

		ImGui::End();
	}

	void AnimationWindow::AnimatorEventPopup(SliceEngine::SliceEngineTypes::Animation& animClip, size_t animClipIndex, SliceEngine::SliceEngineTypes::AnimationKeyFrame& keyFrame)
	{
		if (ImGui::BeginPopupModal("AnimationEventPopup",nullptr))
		{
			if (StringInputHeader(mRegistry, "Function Name: ", "##animEventFuncName", keyFrame.scriptFunc))
			{ }

			if (StringInputHeader(mRegistry, "Param String: ", "##animEventParams", keyFrame.scriptName))
			{ }

			if (ImGui::Button("Save Changes"))
			{
				//ImGui::NeoClearSelection();
				mOpenEventPopup = false;
				LoadDataFromAnimationClip(animClip, animClipIndex);
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
	void AnimationWindow::AnimatorEventPopupCustom(SliceEngine::SliceEngineTypes::Sequence& animClip, size_t animClipIndex, SliceEngine::SliceEngineTypes::AnimationKeyFrame& keyFrame)
	{
		if (ImGui::BeginPopupModal("AnimationEventPopup", nullptr))
		{
			if (StringInputHeader(mRegistry, "Function Name: ", "##animEventFuncName", keyFrame.scriptFunc))
			{
			}

			if (StringInputHeader(mRegistry, "Param String: ", "##animEventParams", keyFrame.scriptName))
			{
			}

			if (ImGui::Button("Save Changes"))
			{
				//ImGui::NeoClearSelection();
				mOpenEventPopup = false;
				LoadDataFromSequenceClip(animClip, animClipIndex);
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void AnimationWindow::AnimatorSRTPopup()
	{
		ImGui::OpenPopup("TrfRow_PopupOptions");
		if (ImGui::BeginPopup("TrfRow_PopupOptions"))
		{
			if (ImGui::Selectable("Add Key"))
			{
				Transform* trf = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Transform>(tmpEnt);

				switch (mOpenSRTVar)
				{
				case 0:
					//mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.push_back({ static_cast<unsigned int>(currentFrame),trf->position });
					InsertNewKey(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform, { static_cast<unsigned int>(currentFrame),trf->position });
					InsertNewKey(customAnimClips[mCurrentClipIndex].transform, { static_cast<unsigned int>(currentFrame),trf->position });
					break;
				case 1:
					//mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation.push_back({ static_cast<unsigned int>(currentFrame),glm::eulerAngles(trf->rotation) });
					InsertNewKey(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation, { static_cast<unsigned int>(currentFrame),glm::eulerAngles(trf->rotation) });
					InsertNewKey(customAnimClips[mCurrentClipIndex].rotation, { static_cast<unsigned int>(currentFrame),glm::eulerAngles(trf->rotation) });
					break;
				case 2:
					//mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale.push_back({ static_cast<unsigned int>(currentFrame),trf->scale });
					InsertNewKey(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale, { static_cast<unsigned int>(currentFrame),trf->scale });
					InsertNewKey(customAnimClips[mCurrentClipIndex].scale, { static_cast<unsigned int>(currentFrame),trf->scale });
					break;
				}
				
				LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
				mOpenTrfOption = false;
				mOpenSRTVar = -1;
			}

			ImGui::EndPopup();
		}
	}
	void AnimationWindow::AnimatorSRTPopupEdit()
	{
		std::string propToDel{};

		ImGui::OpenPopup("EditTrfRow_Popup");
		if (ImGui::BeginPopup("EditTrfRow_Popup"))
		{
			if (ImGui::Selectable("Edit Key"))
			{
				mOpenTrfEditKeyAttrib = true;

				mOpenTrfEdit = false;
			}

			if (ImGui::Selectable("Delete Key"))
			{
				switch (mOpenSRTVarEdit)
				{
				case 0:
					mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.erase(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.begin() + mCurrentEventIndex);
					customAnimClips[mCurrentClipIndex].transform.erase(customAnimClips[mCurrentClipIndex].transform.begin() + mCurrentEventIndex);
					propToDel = "Position";
					break;
				case 1:
					mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation.erase(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation.begin() + mCurrentEventIndex);
					customAnimClips[mCurrentClipIndex].transform.erase(customAnimClips[mCurrentClipIndex].transform.begin() + mCurrentEventIndex);
					propToDel = "Rotation";
					break;
				case 2:
					mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale.erase(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale.begin() + mCurrentEventIndex);
					customAnimClips[mCurrentClipIndex].transform.erase(customAnimClips[mCurrentClipIndex].transform.begin() + mCurrentEventIndex);
					propToDel = "Scale";
					break;
				}
				

				for (auto& propGrp : mPropertyGroups)
				{
					if (propGrp.name.find(propToDel) != std::string::npos)
					{
						for (auto& prop : propGrp.properties)
						{
							std::erase(prop.keys, mCurrentEventIndex);
						}
					}
				}
				LoadDataFromSequenceClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
				mOpenTrfEdit = false;
				mOpenSRTVarEdit = -1;
			}

			ImGui::EndPopup();
		}


		
	}
	void AnimationWindow::AnimatorSRTPopupEditKeyAttrib()
	{
		ImGui::OpenPopup("EditSRT_PopUp");
		if (ImGui::BeginPopupModal("EditSRT_PopUp", nullptr))
		{
			std::vector<std::pair<unsigned int, glm::vec3>>* attrib{ nullptr };
			switch (mOpenSRTVarEdit)
			{
			case 0:
				attrib = &mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform;

				break;
			case 1:
				attrib = &mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].rotation;

				break;
			case 2:
				attrib = &mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].scale;

				break;
			}


			if (DragFloatInputHeader(mRegistry, "X: ", "##animX", (attrib->begin() + mCurrentEventIndex)->second.x)) {}
			if (DragFloatInputHeader(mRegistry, "Y: ", "##animY", (attrib->begin() + mCurrentEventIndex)->second.y)) {}
			if (DragFloatInputHeader(mRegistry, "Z: ", "##animZ", (attrib->begin() + mCurrentEventIndex)->second.z)) {}

			if (ImGui::Button("Save Changes"))
			{
				mOpenTrfEditKeyAttrib = false;
				mOpenSRTVarEdit = -1;
				attrib = nullptr;
				customAnimClips[mCurrentClipIndex] = mCurrentAnimator->curr_anims.animations[mCurrentClipIndex];
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
	void AnimationWindow::UnLoadSequencePkgData(SequencePkgData& animsData, SliceEngine::SliceEngineTypes::SequencePackage& anims)
	{
		for (const auto& name : animsData.animations)
		{
			// do this not metadata file name nogga
			if (mRegistry.GetAssetManager().mFilenameToGUID.find(name) != mRegistry.GetAssetManager().mFilenameToGUID.end())
			{
				std::filesystem::path childPath = mRegistry.GetAssetManager().mAssetDirectory / "Animations" / name;

				SequenceData childAnim{};
				childAnim.DeserializeAsset(childPath);

				SliceEngine::SliceEngineTypes::Sequence addAnim{};
				childAnim.UnLoadSequenceData(addAnim);

				anims.animations.push_back(addAnim);
			}
		}
	}
	void AnimationWindow::InsertNewKey(std::vector<std::pair<unsigned int, glm::vec3>>& vec, std::pair<unsigned int, glm::vec3> key)
	{
		auto it = std::lower_bound(vec.begin(), vec.end(), key,
			[](const std::pair<unsigned int, glm::vec3>& a, const std::pair<unsigned int, glm::vec3>& b) {
				return a.first < b.first;
			});

		vec.insert(it, key);
	}
}
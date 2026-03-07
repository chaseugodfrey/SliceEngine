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

		if (selectionManager->mSelectionType != SelectionType::ENTITY)
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
			tmpEnt = entity;
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

			LoadDataFromAnimClip(customAnimClips[0], mCurrentClipIndex);
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
		endFrame = animClip.num_frames;
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

	void AnimationWindow::LoadDataFromAnimClip(SliceEngine::SliceEngineTypes::Anim& animClip, size_t animClipIdx)
	{
		endFrame = animClip.num_frames;
		startFrame = 0;
		currentFrame = 0;
		mCurrentTime = 0;

		if (!mPropertyGroups.empty())
		{
			/*if (std::strcmp(mPropertyGroups[0].name.c_str(), "Events") == 0)
			{
				mPropertyGroups.erase(mPropertyGroups.begin());
			}

			for (auto& propGrp : mPropertyGroups)
			{
				if(std::strcmp(propGrp.name.c_str(), "Events") == 0)
				{
					std::erase_if(mPropertyGroups, [](AnimationPropertyGroup i) { return std::strcmp(i.name.c_str(), "Events") == 0; });
				}

				if (propGrp.name.find("Position") != std::string::npos)
				{
					std::erase_if(mPropertyGroups, [](AnimationPropertyGroup i) { return i.name.find("Position") != std::string::npos; });
				}
			}*/

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
			if (ImGui::AcceptDragDropPayload("Animations"))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Animations"))
				{
					SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
					//TODO: Rayan does drop stuf f here.
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
					LoadDataFromAnimClip(customAnimClips[mCurrentClipIndex], mCurrentClipIndex);

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
			
			//TODO if currentFrame already has an event. Dont add another one
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
						LoadDataFromAnimClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);

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

				static std::filesystem::path targetAnimsPath = std::filesystem::current_path();

				if (ImGui::Button("Create Animation Package"))
				{
					if (targetAnimsPath.filename() != "Animations")
					{
						targetAnimsPath = targetAnimsPath / "Assets" / "Animations";
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
							if (targetAnimsPath.extension() != ".anims")
							{
								targetAnimsPath += ".anims";
							}

							// like this to save new resource?

							std::string relativeAnimsPath = "Animations/" + newAnimsName + ".anims";

							AnimsData Anims{};
							Anims.SerializeAsset(targetAnimsPath);

							mRegistry.GetAssetManager().CreateResource(targetAnimsPath, nullptr, true);
							mCurrentAnimator->Handle_Anims = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Anims>(mRegistry.GetAssetManager().mFilenameToGUID[relativeAnimsPath]);
							UnLoadAnimsData(Anims, mCurrentAnimator->curr_anims);

							newAnimsName = "";
							targetAnimsPath = std::filesystem::current_path();
							ImGui::CloseCurrentPopup();

							if(mCurrentAnimator->curr_anims.animations.size() > 0)
								LoadDataFromAnimClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
						}
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

				static std::filesystem::path targetAnimPath = std::filesystem::current_path();

				if (ImGui::Button("Add Animation"))
				{
					if (targetAnimPath.filename() != "Animations")
					{
						targetAnimPath = targetAnimPath / "Assets" / "Animations";
						//std::filesystem::current_path(target);

						ImGui::OpenPopup("SaveAnim_Popup");
					}
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
							if (targetAnimPath.extension() != ".anim")
							{
								targetAnimPath += ".anim";
							}

							SliceEngine::SliceEngineTypes::Anim newAnim{};
							newAnim.name = newAnimName;

							std::string relativeAnimPath = "Animations/" + newAnimName + ".anim";

							AnimData animData{};
							animData.LoadAnimData(newAnim);
							animData.SerializeAsset(targetAnimPath);

							mRegistry.GetAssetManager().CreateResource(targetAnimPath, nullptr, true);
							//mCurrentAnimator->Handle_Anims = mRegistry.GetAssetManager().mFilenameToGUID[relativeAnimPath];

							mCurrentAnimator->curr_anims.animations.push_back(newAnim);
							customAnimClips.push_back(newAnim);

							std::optional<std::string> parentName = mRegistry.GetAssetManager().GetFilenameFromGUID(mCurrentAnimator->Handle_Anims.getGUID());
							if (parentName)
							{
								std::filesystem::path parentPath = std::filesystem::current_path() / parentName.value();
								AnimsData parentPkg{};
								parentPkg.DeserializeAsset(parentPath);
								parentPkg.animations.push_back(newAnim.name);
								parentPkg.SerializeAsset(parentPath);
							}

							newAnimName = "";
							targetAnimPath = std::filesystem::current_path();
							ImGui::CloseCurrentPopup();

							LoadDataFromAnimClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
						}
					}

					ImGui::EndPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Save"))
				{
					auto animsFilePath = mRegistry.GetAssetManager().GetFilenameFromGUID(mCurrentAnimator->Handle_Anims.getGUID());
					if (animsFilePath.has_value())
					{
						AnimsData Anims{};
						Anims.LoadAnimsData(mCurrentAnimator->curr_anims);

						Anims.SerializeAsset(mRegistry.GetAssetManager().mAssetDirectory / animsFilePath.value());
					}

					// also add for animations
					for (auto& anim : mCurrentAnimator->curr_anims.animations)
					{
						std::string animFilePath = "Animations/" + anim.name + ".anim";

						AnimData animData{};
						animData.LoadAnimData(anim);

						animData.SerializeAsset(mRegistry.GetAssetManager().mAssetDirectory / animFilePath);
					}
				}

				if (clipSize == 0)
					ImGui::BeginDisabled();
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
				LoadDataFromAnimClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
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

											auto it = std::find_if(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.begin(), mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.end(), [&](const std::pair<unsigned int, glm::vec3>& x)
												{
													return (x.first == static_cast<unsigned int>(key));
												});

											if (it != mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.end())
											{
												mCurrentEventIndex = static_cast<int>(std::distance(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.begin(), it));
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
			ImGui::OpenPopup("EditTrfRow_Popup");
			if (ImGui::BeginPopup("EditTrfRow_Popup"))
			{
				if (ImGui::Selectable("Delete Key"))
				{
					mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.erase(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.begin() + mCurrentEventIndex);

					for (auto& propGrp : mPropertyGroups)
					{
						if (propGrp.name.find("Position") != std::string::npos)
						{
							for (auto& prop : propGrp.properties)
							{
								std::erase(prop.keys, mCurrentEventIndex);
							}
						}
					}
					LoadDataFromAnimClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
					mOpenTrfEdit = false;
				}

				ImGui::EndPopup();
			}
		}

		if (mOpenTrfOption)
		{
			ImGui::OpenPopup("TrfRow_PopupOptions");
			if (ImGui::BeginPopup("TrfRow_PopupOptions"))
			{
				if (ImGui::Selectable("Add Key"))
				{
					mCurrentAnimator->curr_anims.animations[mCurrentClipIndex].transform.push_back({ static_cast<unsigned int>(currentFrame),{0.f,0.f,0.f} });
					LoadDataFromAnimClip(mCurrentAnimator->curr_anims.animations[mCurrentClipIndex], mCurrentClipIndex);
					mOpenTrfOption = false;
				}

				ImGui::EndPopup();
			}
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
							mCurrentTime += dt;
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

			ImGui::EndPopup();
		}
	}
	void AnimationWindow::AnimatorEventPopupCustom(SliceEngine::SliceEngineTypes::Anim& animClip, size_t animClipIndex, SliceEngine::SliceEngineTypes::AnimationKeyFrame& keyFrame)
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
				LoadDataFromAnimClip(animClip, animClipIndex);
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
	void AnimationWindow::UnLoadAnimsData(AnimsData& animsData, SliceEngine::SliceEngineTypes::Anims& anims)
	{
		for (const auto& name : animsData.animations)
		{
			// do this not metadata file name nogga
			if (mRegistry.GetAssetManager().mFilenameToGUID.find(name) != mRegistry.GetAssetManager().mFilenameToGUID.end())
			{
				std::filesystem::path childPath = mRegistry.GetAssetManager().mAssetDirectory / "Animations" / name;

				AnimData childAnim{};
				childAnim.DeserializeAsset(childPath);

				SliceEngine::SliceEngineTypes::Anim addAnim{};
				childAnim.UnLoadAnimData(addAnim);

				anims.animations.push_back(addAnim);
			}
		}
	}
}
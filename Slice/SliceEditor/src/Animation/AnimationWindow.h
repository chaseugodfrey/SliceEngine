#ifndef ANIMATION_WINDOW_H
#define ANIMATION_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;

	struct AnimationProperty
	{
		std::string name;
		std::vector<ImGui::FrameIndexType> keys;
		int selectedKeyIndex = -1;
	};

	// Use ImGui::BeginNeoGroup() to visualize properties
	struct AnimationPropertyGroup
	{
		std::string name;
		std::vector<AnimationProperty> properties;
		bool isOpen;
	};

	class AnimationWindow : public EditorWindow
	{
		ImGui::FrameIndexType startFrame{0};
		ImGui::FrameIndexType endFrame{30};
		ImGui::FrameIndexType currentFrame{0};

		std::vector<AnimationPropertyGroup> mPropertyGroups;

		SliceEngine::Timeline mTimeline;
		float mCurrentTime{ 0.0f };
		ImGuiNeoSequencerFlags mSequencerFlags;

		//
		SliceEngine::Animator* mCurrentAnimator{ nullptr };
		//SliceEngine::Transform* mCurrentTransform;

		entt::entity tmpEnt;
		

		std::vector<SliceEngine::SliceEngineTypes::Animation* > animationClips;
		std::vector<SliceEngine::SliceEngineTypes::Sequence> customAnimClips;
		size_t mCurrentClipIndex{ 0 };
		size_t mCurrentEventIndex{ 0 };
		size_t mCurrentKeyIndex{ 0 };
		bool mOpenEventPopup{};
		bool mOpenEventOption{ false };
		bool mOpenTrfOption{};
		int mOpenSRTVar{ -1 };
		int mOpenSRTVarEdit{ -1 };
		bool mOpenTrfEdit{};
		bool mOpenTrfEditKeyAttrib{};

		bool CheckForAnimator();
		void LoadDataFromAnimator(SliceEngine::Animator* component, entt::entity entity);
		void LoadDataFromAnimationClip(SliceEngine::SliceEngineTypes::Animation& animClip, size_t animCipIdx);
		void LoadDataFromSequenceClip(SliceEngine::SliceEngineTypes::Sequence& animClip, size_t animCipIdx);
		void LoadPropertyGroup(entt::entity entity, SliceEngine::SceneGraph& scene_graph);
		void ClearData();

		void UpdateTransform(SliceEngine::SliceEngineTypes::Animation* animClip,float time);
		void UpdateBoneScene(Entity ent);
		void UpdateBones();
		void AnimatorEventPopup(SliceEngine::SliceEngineTypes::Animation& animClip, size_t animClipIndex, SliceEngine::SliceEngineTypes::AnimationKeyFrame& keyFrame);
		void AnimatorEventPopupCustom(SliceEngine::SliceEngineTypes::Sequence& animClip, size_t animClipIndex, SliceEngine::SliceEngineTypes::AnimationKeyFrame& keyFrame);

		void AnimatorSRTPopup();
		void AnimatorSRTPopupEdit();
		void AnimatorSRTPopupEditKeyAttrib();

		void UnLoadSequencePkgData(SequencePkgData& animsData, SliceEngine::SliceEngineTypes::SequencePackage& anims);
		void InsertNewKey(std::vector<std::pair<unsigned int, glm::vec3>>& vec, std::pair<unsigned int, glm::vec3> key);

	public:
		AnimationWindow(Registry& reg) : EditorWindow(reg) {};
		~AnimationWindow();
		void Init() override;
		void Draw() override final;
	};
}

#endif
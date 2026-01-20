/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        EditorCommonTypes.h

 author:	  Chase Rodrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines common types used across the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_COMMON_TYPES_H
#define EDITOR_COMMON_TYPES_H

#include "AssetManager/AssetTypes.h"

namespace SliceEditor
{

	enum class ManagerID
	{
		CONTENTBROWSER,
		HIERARCHY,
		INSPECTOR,
		SCENEVIEW,
		GAMEVIEW,
		ANIMATOR,
		NAVMESH,
		LIGHTING,
		AUDIO,
		PROFILER,
		CONSOLE,
		ID_MAX
	};

	enum class WindowID
	{
		CONTENTBROWSER,
		HIERARCHY,
		INSPECTOR,
		SCENEVIEW,
		GAMEVIEW,
		ANIMATOR,
		NAVMESH,
		LIGHTING,
		AUDIO,
		PROFILER,
		CONSOLE,
		ID_MAX
	};

	enum class SelectionType : int
	{
		NONE = 0,
		FOLDER = 1,
		ENTITY = 2,
		TEXTURE = 3,
		MODEL = 4,
		AUDIO = 5,
		SCENE = 6,
		SHADER = 7,
		MATERIAL = 8,
		PREFAB = 9,
		PREFAB_ENTITY = 10,
		CONTROLLER = 11,
		STATE = 12,
		TRANSITION = 13,
		TEXTFILE = 14,
		MIXED = 15,
		UNSUPPORTED = 16
	};

	struct SelectionNode
	{
		SelectionType type;
		bool isSelected;
	};

	const std::unordered_map<std::string, SelectionType> mExtensionToSelectionType =
	{
			{"",  SelectionType::FOLDER },
			{".png",  SelectionType::TEXTURE },
			{".jpg", SelectionType::TEXTURE},
			{".jpeg", SelectionType::TEXTURE},
			{".dds", SelectionType::TEXTURE},
			{".tga", SelectionType::TEXTURE},
			{".gif", SelectionType::TEXTURE},
			{".obj", SelectionType::MODEL},
			{".fbx", SelectionType::MODEL},
			{".wav", SelectionType::AUDIO},
			{".mp3", SelectionType::AUDIO},
			{".ogg", SelectionType::AUDIO},
			{".scene", SelectionType::SCENE},
			{".shader", SelectionType::SHADER},
			{".mat", SelectionType::MATERIAL},
			{".txt", SelectionType::TEXTFILE},
			{".prefab", SelectionType::PREFAB},
			{".controller", SelectionType::CONTROLLER}
	};

	const std::unordered_map<SelectionType, std::string> mSelectionTypeToString =
	{
			{ SelectionType::ENTITY, "GameObject"},
			{ SelectionType::FOLDER, "Folder"},
			{ SelectionType::TEXTURE, "Texture"},
			{ SelectionType::MATERIAL, "Material"},
			{ SelectionType::MODEL, "Model"},
			{ SelectionType::PREFAB, "Prefab"},
			{ SelectionType::PREFAB_ENTITY, "Prefab"},
			{ SelectionType::TEXTFILE, "File"},
			{ SelectionType::SCENE, "Scene"},
			{ SelectionType::SHADER, "Shader"},
			{ SelectionType::AUDIO, "Audio"},
			{ SelectionType::MIXED, "Mixed"},
			{ SelectionType::NONE, "None"},
			{ SelectionType::CONTROLLER, "Controller"}
	};

	struct EntityNode : SelectionNode
	{
		//std::string name;
		entt::entity entity = entt::null;
		bool isPrefab = false;

		EntityNode()
			: entity(entt::null), isPrefab(false)
		{
			type = SelectionType::ENTITY;
			isSelected = false;
		}

		EntityNode(entt::entity ent) : entity(ent)
		{
			type = SelectionType::ENTITY;
			isSelected = false;
			isPrefab = false;
		}
	};

	/*struct PrefabNode : SelectionNode
	{
		PrefabNode()
			: entity(entt::null)
		{
			type = SelectionType::PREFAB_ENTITY;
			isSelected = false;
		}

		PrefabNode(entt::entity ent) : entity(ent)
		{
			type = SelectionType::PREFAB;
			isSelected = false;
		}
		entt::entity entity = entt::null;
		std::vector<PrefabNode> children;
	};*/

	struct DirectoryNode : SelectionNode //Content Browser
	{
		std::string fileName;
		std::filesystem::path fullPath;
		std::filesystem::path relativePath;
		DirectoryNode* parent = nullptr;
		std::map<std::string, DirectoryNode> children;
		bool isDirectory = false;
	};

	struct StateNode : SelectionNode
	{
		int id{};
		int in_id{};
		int out_id{};
		std::vector<int> transitionIds;

		std::string name{};
		ImVec2 position{};

		StateNode()
		{
			type = SelectionType::STATE;
		}
	};

	struct TransitionLinkNode : SelectionNode
	{
		int id{};
		int source_id{};
		int target_id{};
		int source_out_id{};
		int target_in_id{};

		TransitionLinkNode()
		{
			type = SelectionType::TRANSITION;
		}
	};

	struct AnimatorData
	{
		std::unique_ptr<StateMachineData> mStateMachineAsset;
		std::unordered_map<int, StateNode> mStateNodes;
		std::unordered_map<int, TransitionLinkNode> mTransitionNodes;
		std::unordered_map<std::string, int> mNameToStateID;
		
		using State = SliceEngine::SliceEngineTypes::State;
		using Transition = SliceEngine::SliceEngineTypes::Transition;
		using Parameters = decltype(StateMachineData::parameters);

		bool empty() const
		{
			return mStateMachineAsset == nullptr;
		}
		
		void reset()
		{
			mStateMachineAsset.reset();
			mStateNodes.clear();
			mTransitionNodes.clear();
			mNameToStateID.clear();
		}

		bool Load(const std::filesystem::path filepath)
		{
			StateMachineData data{};
			if (!data.DeserializeAsset(filepath))
			{
				SLICE_LOG_ERROR(".controller filepath does not exist!");
				return false;
			}

			mStateMachineAsset = std::make_unique<StateMachineData>(data);

			int nodeId{ 2 };
			int linkId{ 0 };
			auto& stateMap = data.stateMap;

			for (auto& [name, state] : stateMap)
			{
				StateNode node{};
				node.id = nodeId;
				node.in_id = nodeId * 2;
				node.out_id = node.in_id + 1;
				node.name = name;
				node.position = ImVec2(state.mNodePos);

				mStateNodes.emplace(nodeId, node);
				mNameToStateID.emplace(name, nodeId);
				nodeId++;
			}

			for (auto& [sourceId, sourceNode] : mStateNodes)
			{
				auto& sourceState = stateMap.at(sourceNode.name);

				for (auto& transition : sourceState.transitions)
				{
					transition.id = linkId;
					sourceNode.transitionIds.push_back(linkId);

					auto targetStateName = transition.targetState;

					auto it = mNameToStateID.find(targetStateName);
					if (it == mNameToStateID.end())
						continue;

					auto targetId = mNameToStateID.at(targetStateName);
					auto& targetNode = mStateNodes.at(targetId);

					TransitionLinkNode link;
					link.id = linkId;
					link.source_id = sourceId;
					link.target_id = targetId;
					link.source_out_id = sourceNode.out_id;
					link.target_in_id = targetNode.in_id;

					mTransitionNodes.emplace(linkId, link);
					linkId++;
				}
			}

			return true;
		}

		std::optional<std::reference_wrapper<StateNode>> GetStateNode(int state_id)
		{
			auto it1 = mStateNodes.find(state_id);
			if (it1 == mStateNodes.end())
				return std::nullopt;

			return it1->second;
		}

		std::optional<std::reference_wrapper<StateNode>> GetStateNode(std::string name)
		{
			auto it1 = mNameToStateID.find(name);
			if (it1 == mNameToStateID.end())
				return std::nullopt;

			return GetStateNode(it1->second);
		}

		std::optional<std::reference_wrapper<State>> GetState(std::string name)
		{
			auto it1 = mStateMachineAsset->stateMap.find(name);
			if (it1 == mStateMachineAsset->stateMap.end())
				return std::nullopt;

			return it1->second;
		}

		std::optional<std::reference_wrapper<State>> GetState(int state_id)
		{
			auto node = GetStateNode(state_id);
			if (!node.has_value())
				return std::nullopt;

			auto it1 = mStateMachineAsset->stateMap.find(node.value().get().name);
			if (it1 == mStateMachineAsset->stateMap.end())
				return std::nullopt;

			return it1->second;
		}

		std::optional<std::reference_wrapper<TransitionLinkNode>> GetTransitionNode(int transition_id)
		{
			auto it1 = mTransitionNodes.find(transition_id);
			if (it1 == mTransitionNodes.end())
				return std::nullopt;

			return it1->second;
		}

		std::optional<std::reference_wrapper<State>> GetState(const Transition& transition)
		{
			 auto link = GetTransitionNode(transition.id);
			 if (!link.has_value())
				 return std::nullopt;

			 return GetState(link->get().source_id);
		}

		static std::optional<std::reference_wrapper<Transition>> GetTransition(State& state, int transition_id)
		{
			auto& transitions = state.transitions;
			auto it = std::find_if(transitions.begin(), transitions.end(), [&](const Transition transition) { return transition.id == transition_id; });

			if (it == transitions.end())
				return std::nullopt;

			return *it;
		}

		std::optional<std::reference_wrapper<Transition>> GetTransition(int transition_id)
		{
			auto link = GetTransitionNode(transition_id);
			if (!link.has_value())
				return std::nullopt;

			auto state = GetState(link->get().source_id);
			if (!state.has_value())
				return std::nullopt;

			return GetTransition(state->get(), transition_id);
		}

		std::optional<std::reference_wrapper<Parameters>> GetParameters()
		{
			return mStateMachineAsset->parameters;
		}

	};

	struct DroppedFile //Dropped File (From File Explorer to Editor)
	{
		AssetType assetType;
		std::unique_ptr<MetaData> metaData;
		std::filesystem::path filePath;
	};

	// ANIMATIONS



	// THEMES

	enum EditorThemeType : int
	{
		DARK = 0,
		LIGHT = 1,
		MICROSOFT = 2
	};

	constexpr std::array<const char*, 3> EditorThemes =
	{
		"Dark",
		"Light",
		"Microsoft"
	};

	// PREFERENCES

	struct Preferences
	{
		struct Theme
		{
			EditorThemeType ID;
		};

		struct Scene
		{
			SliceEngine::GUID startingID;
			SliceEngine::GUID lastID;
		};

		unsigned int version;
		Theme theme;
		Scene scene;
	};

	struct testtest
	{
		float test;
	};
}

#endif
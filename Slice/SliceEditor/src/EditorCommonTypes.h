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

#include <memory>
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
		SHADERGRAPH = 15,
		SHADER_STATE = 16,
		SHADER_FUNCTION_STATE = 17,
		SHADER_LINK_STATE = 18,
		MIXED = 19,
		UNSUPPORTED = 20
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
			{".glb", SelectionType::MODEL},
			{".wav", SelectionType::AUDIO},
			{".mp3", SelectionType::AUDIO},
			{".ogg", SelectionType::AUDIO},
			{".scene", SelectionType::SCENE},
			{".shader", SelectionType::SHADER},
			{".cshader", SelectionType::SHADERGRAPH},
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
			{ SelectionType::SHADERGRAPH, "CShader"},
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
		bool seen = false; //For editor Hierarchy to check if it should be removed or not
		bool nodeOpen = false;
		bool isScriptSelected = false;

		EntityNode()
			: entity(entt::null), isPrefab(false), seen(false), isScriptSelected(false)
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

	struct CategoryNode : SelectionNode
	{
		std::string name;
		SelectionType type;
	};


	struct StateNode : SelectionNode
	{
		int id{};
		int in_id{};
		int out_id{};
		std::vector<int> transitionIds;

		std::string name{};

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
		std::unordered_map<uint16_t, std::unique_ptr<StateNode>> mStateNodes;
		std::unordered_map<uint16_t, std::unique_ptr<TransitionLinkNode>> mTransitionNodes;
		std::unordered_map<std::string, uint16_t> mNameToStateID;

		using State = SliceEngine::SliceEngineTypes::State;
		using Transition = SliceEngine::SliceEngineTypes::Transition;
		using Parameters = decltype(StateMachineData::parameters);

		void create_state(std::string newName)
		{
			std::string state_name = newName;
			int repeat = 1;

			//while (auto it = mNameToStateID.find(state_name) != mNameToStateID.end())
			if(state_name.empty())
			{
				state_name = "New State " + std::to_string(repeat);
				//repeat++;
			}

			while (auto it = mNameToStateID.find(state_name) != mNameToStateID.end())
			{

			}

			State state{};
			state.stateName = state_name;
			state.mNodePos = glm::vec2(0.0f, 0.0f);

			mStateMachineAsset->stateMap.emplace(state_name, state);

			create_state_node(state_name);
		}

		void create_state_node(std::string name)
		{
			auto node = std::make_unique<StateNode>();
			node->id = static_cast<int>(mStateNodes.size());
			node->in_id = node->id * 2;
			node->out_id = node->in_id + 1;

			for (auto& [nm, id] : mNameToStateID)
			{
				if (nm == name)
				{
					name += " copy";
					break;
				}
			}

			node->name = name;
			uint16_t id = static_cast<uint16_t>(node->id);
			mStateNodes.emplace(id, std::move(node));
			mNameToStateID.emplace(name, id);
		}

		void create_link(StateNode const& source, StateNode const& target)
		{
			auto link = std::make_unique<TransitionLinkNode>();
			link->id = static_cast<int>(mTransitionNodes.size());
			link->source_id = source.id;
			link->target_id = target.id;
			link->source_out_id = source.out_id;
			link->target_in_id = target.in_id;

			mTransitionNodes.emplace(static_cast<uint16_t>(link->id), std::move(link));

		}

		void check_default()
		{
			if (!mStateNodes.empty())
			{
				if (mStateNodes.at(0)->name == "Entry" && mStateNodes.at(1)->name == "Exit")
					return;
			}

			mStateNodes.clear();
			create_default();
		}

		void create_default()
		{
			create_state_node("Entry");
			create_state_node("Exit");
		}

		void set_position(int state_id, ImVec2 pos)
		{
			if (state_id == 0)
			{
				mStateMachineAsset->entryPosition = glm::vec2(pos.x, pos.y);
				return;
			}

			else if (state_id == 1)
			{
				mStateMachineAsset->exitPosition = glm::vec2(pos.x, pos.y);
				return;
			}

			else
			{
				auto& state_map = mStateMachineAsset->stateMap;
				auto it = mStateNodes.find(static_cast<const unsigned short>(state_id));
				if (it != mStateNodes.end())
				{
					state_map.at(it->second->name).mNodePos = glm::vec2(pos.x, pos.y);
				}
			}
		}

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

			data.InitMetaData(filepath, AssetType::Controller, "Controller");
			mStateMachineAsset = std::make_unique<StateMachineData>(data);

			auto& stateMap = mStateMachineAsset->stateMap;

			create_default();

			for (auto& [name, state] : stateMap)
			{
				create_state_node(name);
			}

			for (auto& [sourceId, sourceNode] : mStateNodes)
			{
				if (sourceNode->name == "Entry")
				{
					auto it = mNameToStateID.find(data.entryState);
					if (it == mNameToStateID.end())
						continue;
					
					create_link(*sourceNode, *mStateNodes.at(it->second));

					continue;
				}

				else if (sourceNode->name == "Exit")
				{
					continue;
				}

				auto& sourceState = stateMap.at(sourceNode->name);
				
				for (auto& transition : sourceState.transitions)
				{
					auto targetStateName = transition.targetState;

					auto it = mNameToStateID.find(targetStateName);
					if (it == mNameToStateID.end())
						continue;

					auto targetId = mNameToStateID.at(targetStateName);
					auto& targetNode = *mStateNodes.at(targetId);

					create_link(*sourceNode, targetNode);
					transition.id = static_cast<int>(mTransitionNodes.size()) - 1;
					sourceNode->transitionIds.push_back(transition.id);
				}
			}

			return true;
		}

		void LoadFromAsset(const SliceEngine::SliceEngineTypes::StateMachine& stateMachine)
		{
			StateMachineData data{};

			data.stateMap = stateMachine.stateMap;
			data.entryPosition = stateMachine.entryPosition;
			data.exitPosition = stateMachine.exitPosition;

			mStateMachineAsset = std::make_unique<StateMachineData>(data);

			auto& stateMap = mStateMachineAsset->stateMap;

			mStateNodes.clear();
			mNameToStateID.clear();
			create_default();

			for (auto& [name, state] : stateMap)
			{
				create_state_node(name);
			}

			for (auto& [sourceId, sourceNode] : mStateNodes)
			{
				if (sourceNode->name == "Entry")
				{
					auto it = mNameToStateID.find(data.entryState);
					if (it == mNameToStateID.end())
						continue;

					create_link(*sourceNode, *mStateNodes.at(it->second));

					continue;
				}

				else if (sourceNode->name == "Exit")
				{
					continue;
				}

				auto& sourceState = stateMap.at(sourceNode->name);

				for (auto& transition : sourceState.transitions)
				{
					auto targetStateName = transition.targetState;

					auto it = mNameToStateID.find(targetStateName);
					if (it == mNameToStateID.end())
						continue;

					auto targetId = mNameToStateID.at(targetStateName);
					auto& targetNode = *mStateNodes.at(targetId);

					create_link(*sourceNode, targetNode);
					transition.id = static_cast<int>(mTransitionNodes.size() - 1);
					sourceNode->transitionIds.push_back(transition.id);
				}
			}
		}

		std::optional<std::reference_wrapper<StateNode>> GetStateNode(int state_id)
		{
			auto it1 = mStateNodes.find(static_cast<const unsigned short>(state_id));
			if (it1 == mStateNodes.end())
				return std::nullopt;

			return *(it1->second);
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
			auto it1 = mTransitionNodes.find(static_cast<const unsigned short>(transition_id));
			if (it1 == mTransitionNodes.end())
				return std::nullopt;

			return *(it1->second);
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
		bool toRecompile = true;
	};

	// ANIMATIONS



	// THEMES

	enum EditorThemeType : int
	{
		DARK = 0,
		LIGHT = 1,
		MICROSOFT = 2,
		CLASSICSTEAM = 3,
		GREENLEAF = 4,
		DARCULA = 5,
		DISCORDDARK = 6,
		DEEPDARK = 7,
		MOONLIGHT = 8,
		EXCELLENCY = 9,
		REST = 10
	};

	constexpr std::array<const char*, 11> EditorThemes =
	{
		"Dark",
		"Light",
		"Microsoft",
		"ClassicSteam",
		"GreenLeaf",
		"Darcula",
		"DiscordDark",
		"DeepDark",
		"Moonlight",
		"Excellency",
		"Rest"
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

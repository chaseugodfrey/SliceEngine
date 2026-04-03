#include <pch.h>
#include "AnimatorWindow.h"
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include "Inspector/ComponentPropertiesGUI.h"
#include <Session/SessionManager.h>

namespace SliceEditor
{
	AnimatorWindow::~AnimatorWindow()
	{

	} 

	void AnimatorWindow::Init()
	{
		mSessionManager = mRegistry.GetManager<SessionManager>("Session");
		//mAnimatorData = mSessionManager->GetAnimatorData();
		EventManager::GetInstance()->Subscribe<ClearSelectionEvent, &AnimatorWindow::ClearSelectionSubscribe>(this);
		EventManager::GetInstance()->Subscribe<OnAnimatorChangedEvent,&AnimatorWindow::ReloadAnimatorData>(this);

		//ImNodes::PushColorStyle(ImNodesCol_NodeBackground, )
		//entryNode.id = 0;
		//entryNode.in_id = -1;
		//entryNode.out_id = 1;
		//entryNode.name = "Entry";

		//exitNode.id = 1;
		//exitNode.in_id = 2;
		//exitNode.out_id = -1;
		//exitNode.name = "Exit";
	}

	void AnimatorWindow::Draw()
	{
		CheckForAnimator();
		
		ImGui::Begin("Animator");
		DrawMenuBar();
		DrawParameters();
		ImGui::SameLine();
		DrawNodeEditor();
		DrawPostEditorElements();		
		ImGui::End();
	}

	void AnimatorWindow::DrawMenuBar()
	{
		if (ImGui::Button("Save"))
		{
			if (!mAnimatorData)
				return;

			mAnimatorData->mStateMachineAsset->SerializeAsset();
		}

		ImGui::SameLine();

		static std::filesystem::path targetControllerPath = std::filesystem::current_path();

		if (ImGui::Button("Save As"))
		{
			if (!mAnimatorData)
				return;

			if (targetControllerPath.filename() != "Controllers")
			{
				targetControllerPath = targetControllerPath / "Assets" / "Controllers";
				//std::filesystem::current_path(target);
			}

			ImGui::OpenPopup("SaveAs_Popup");

		}
		
		if (ImGui::BeginPopupModal("SaveAs_Popup", nullptr))
		{
			static std::string newFileName = "";
			if (StringInputHeader(mRegistry, "New File Name: ", "##newFileController", newFileName))
			{

			}

			if (ImGui::Button("Save Changes"))
			{
				targetControllerPath = targetControllerPath / newFileName;
				if (targetControllerPath.extension() != ".controller")
				{
					targetControllerPath += ".controller";
				}
				mAnimatorData->mStateMachineAsset->SerializeAsset(targetControllerPath);

				//mRegistry.GetAssetManager().CreateDefaultAsset(targetControllerPath, )

				newFileName = "";
				targetControllerPath = std::filesystem::current_path();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void AnimatorWindow::CheckForAnimator()
	{
		// Check if any entities selected
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		if (!(selectionManager->mSelectionType == SelectionType::ENTITY || selectionManager->mSelectionType == SelectionType::PREFAB_ENTITY))
			return;

		auto& nodes = selectionManager->GetSelectedNodes();
		Entity entity = entt::null;

		// if entities present
		if (nodes.size() > 0)
		{
			EntityNode* entityNode = static_cast<EntityNode*>(*nodes.begin());
			entity = entityNode->entity;

			// check if first entity has animator component
			auto anim = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Animator>(entity);

			// if anim exists
			if (anim)
			{
				tmpEnt = entity;
				// if current animator is null or mismatch
				// ignore if anim == mCurrentAnimator
				// either case, return true
				if (!mCurrentAnimator || anim != mCurrentAnimator)
				{
					LoadDataFromAnimator(anim, entity);
				}
			}
		}
	}

	void AnimatorWindow::ClearSelectionSubscribe(ClearSelectionEvent e)
	{
		ClearSelection();
	}

	void AnimatorWindow::ClearSelection()
	{
		ImNodes::ClearNodeSelection();
		ImNodes::ClearLinkSelection();
	}

	void AnimatorWindow::DrawParameters()
	{
		ImGui::BeginChild("##left_region", ImVec2(0.3f * ImGui::GetWindowWidth(), 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);

		ImGui::SeparatorText("Parameters");
		/*if (ImGui::Button("+##add_param"))
		{
			ImGui::OpenPopup("AddParam_Popup");
		}*/

		if (ImGui::BeginPopupContextItem("AddParam_Popup"))
		{
			if (ImGui::Selectable("Bool"))
			{

			}

			if (ImGui::Selectable("Int"))
			{

			}

			if (ImGui::Selectable("Float"))
			{

			}

			if (ImGui::Selectable("Trigger"))
			{

			}

			ImGui::EndPopup();
		}

		if (mAnimatorData != nullptr)
		{
			if(!mAnimatorData->empty())
			{
				// Removed LoadFromAsset every frame to prevent selection flickering
				// mAnimatorData->LoadFromAsset(mCurrentAnimator->stateMachine.EFSM);
				int param_id{};
				for (auto& [name, param] : mAnimatorData->mStateMachineAsset->parameters)
				{
					std::string param_label_id = "##param" + name + std::to_string(param_id);
					std::string param_name = name;
					ImGui::SetNextItemWidth(125.0f);
					if (ImGui::InputText(param_label_id.c_str(), &param_name))
					{

					}

					ImGui::SameLine(0.0f, 100.0f);

					if (param.is_type<float>())
					{
						ImGui::SetNextItemWidth(150.0f);
						DragFloatInputHeader(mRegistry, "", (param_label_id + "_float").c_str(), param.get_value<float>());
					}

					else if (param.is_type<int>())
					{
						ImGui::SetNextItemWidth(150.0f);
						DragIntInputHeader(mRegistry, "", (param_label_id + "_int").c_str(), param.get_value<int>());
					}

					else if (param.is_type<bool>())
					{
						BoolInputHeader(mRegistry, "", (param_label_id + "_bool").c_str(), param.get_value<bool>());
					}
				}
			}
		}

		ImGui::EndChild();
	}

	void AnimatorWindow::DrawEntryNode()
	{
		ImNodes::PushColorStyle(
			ImNodesCol_NodeBackground, IM_COL32(0, 180, 0, 255));

		ImNodes::PushColorStyle(
			ImNodesCol_NodeBackgroundHovered, IM_COL32(0, 200, 0, 255));

		ImNodes::PushColorStyle(
			ImNodesCol_NodeBackgroundSelected, IM_COL32(0, 225, 0, 255));

		DrawStateNode(&entryNode);

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();

	}

	void AnimatorWindow::DrawExitNode()
	{
		ImNodes::PushColorStyle(
			ImNodesCol_NodeBackground, IM_COL32(180, 0, 0, 255));

		ImNodes::PushColorStyle(
			ImNodesCol_NodeBackgroundHovered, IM_COL32(200, 0, 0, 255));

		ImNodes::PushColorStyle(
			ImNodesCol_NodeBackgroundSelected, IM_COL32(225, 0, 0, 255));

		DrawStateNode(&exitNode);

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();

	}

	void AnimatorWindow::DrawNodeEditor()
	{
		ImGui::BeginChild("##right_region", ImVec2(0.7f * ImGui::GetWindowWidth(), 0.0f), ImGuiChildFlags_Borders);

		ImNodes::BeginNodeEditor();

		if (mAnimatorData != nullptr)
		{
			if (!mAnimatorData->empty())
			{
				// Removed LoadFromAsset every frame to prevent selection flickering
				// mAnimatorData->LoadFromAsset(mCurrentAnimator->stateMachine.EFSM);

				for (auto& [id, node] : mAnimatorData->mStateNodes)
				{
					DrawStateNode(node.get());
				}

				for (auto& [id, link] : mAnimatorData->mTransitionNodes)
				{
					DrawTransitionLinkNode(link.get());
				}
			}
		}

		//DrawEntryNode();
		//DrawExitNode();
		DrawPostEditorElements();

		if (mAnimatorData)
			ImNodes::MiniMap();

		ImNodes::EndNodeEditor();

		CheckStateNode();

		CheckTransitionLinkNode();


		ImGui::EndChild();
	}

	void AnimatorWindow::DrawPostEditorElements()
	{
		if (mAnimatorData != nullptr)
		{
			if (!mAnimatorData->empty())
			{
				bool shouldOpenModal = false;

				//// Check for inputs for popups
				/*for (auto& [id, node] : mAnimatorData->mStateNodes)
				{
					if (CheckStateInput(&node))
					{
						mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(&node);
					}
				}*/

				//for (auto& [id, link] : mAnimatorData->mTransitionNodes)
				//{
				//	if (CheckLinkInput(&link))
				//	{
				//		mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(&link);
				//	}
				//}

				if (ImNodes::IsEditorHovered())
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						ImGui::OpenPopup("NodeEditor_Popup");
					}
				}

				if (ImGui::BeginPopup("Node_Popup"))
				{
					if (ImGui::Selectable("Make Entry State"))
					{

					}

					ImGui::EndPopup();
				}

				/*if (ImGui::BeginPopup("Link_Popup"))
				{
					if (ImGui::Selectable("Delete"))
					{

					}

					ImGui::EndPopup();
				}*/

				if (ImGui::BeginPopup("NodeEditor_Popup"))
				{
					if (ImGui::Selectable("Create Node"))
					{
						/*static bool openCreateNode = true;*/
						shouldOpenModal = true;
					}

					ImGui::EndPopup();
				}

				if (shouldOpenModal)
				{
					ImGui::OpenPopup("CreateNewNodeEditor_Popup");
				}

				if (ImGui::BeginPopupModal("CreateNewNodeEditor_Popup", nullptr))
				{
					static std::string createNode = "";
					if (StringInputHeader(mRegistry, "State Name: ", "##newNodeAdd", createNode))
					{

					}

					if (ImGui::Button("Add"))
					{

						CreateNode(createNode);

						createNode = "";
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}
		}
	}

	void AnimatorWindow::SaveAnimatorData()
	{
		//auto& sm = mAnimatorData->mStateMachineAsset;

		//sm->entryPosition = ImNodes::GetNodeEditorSpacePos(0);
		//sm->exitPosition = ImNodes::GetNodeEditorSpacePos(1);

		//for (auto& [id, node] : mAnimatorData->mStateNodes)
		//{
		//	if (id == 0 || id == 1)
		//		continue;

		//	
		//}

	}

	void AnimatorWindow::ReloadAnimatorData(OnAnimatorChangedEvent e)
	{
		//auto& animator = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Animator>(e.ent);
		LoadDataFromAnimator(mCurrentAnimator,e.ent);
	}

	bool AnimatorWindow::CheckStateInput(StateNode* node)
	{
		int id = node->id;
		/*if (ImNodes::IsNodeHovered(&id))
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("Node_Popup");
			}
		}*/
		return ImNodes::IsNodeSelected(id);
	}

	bool AnimatorWindow::CheckLinkInput(TransitionLinkNode* node)
	{
		int id = static_cast<TransitionLinkNode*>(node)->id;
		//if (ImNodes::IsLinkHovered(&id))
		//{
		//	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		//	{
		//		ImGui::OpenPopup("Link_Popup");
		//	}
		//}

		return ImNodes::IsLinkSelected(id);
	}

	void AnimatorWindow::DrawStateNode(StateNode* node)
	{
		ImNodes::BeginNode(node->id);
		//ImNodes::BeginNodeTitleBar();
		//ImGui::TextUnformatted(name.c_str());
		//ImNodes::EndNodeTitleBar();

		if (node->in_id > 0)
		{
			ImNodes::BeginInputAttribute(node->in_id);
			ImGui::Text("");
			ImNodes::EndInputAttribute();
			ImGui::SameLine();
		}

		ImGui::TextUnformatted(node->name.c_str());

		if (node->out_id > 0)
		{
			ImGui::SameLine();
			ImNodes::BeginOutputAttribute(node->out_id);
			//ImGui::Indent(40);
			ImGui::Text("");
			ImNodes::EndOutputAttribute();
		}

		ImNodes::EndNode();

		if (ImNodes::IsNodeSelected(node->id))
		{
			if (ImGui::IsWindowFocused())
			{
				SelectNode(static_cast<uint16_t>(node->id));
			}
		}

		auto pos = ImNodes::GetNodeEditorSpacePos(node->id);
		mAnimatorData->set_position(node->id, pos);
	}

	void AnimatorWindow::CheckStateNode()
	{
		int hoveredNodeId = -1;
		static int nodeToDelete = -1;
		bool openLinkPopup = false;

		if(ImNodes::IsNodeHovered(&hoveredNodeId))
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				nodeToDelete = hoveredNodeId;
				
				ImGui::OpenPopup("EditNode_Popup");
			}
		}

		if (ImGui::BeginPopup("EditNode_Popup"))
		{
			if (ImGui::Selectable("Add Link"))
			{
				if (nodeToDelete != -1)
				{
					//ImGui::OpenPopup("NewLink_Popup");
					openLinkPopup = true;
				}
			}
			if (ImGui::Selectable("Delete State"))
			{
				if (nodeToDelete != -1)
				{
					DeleteNode(static_cast<uint16_t>(nodeToDelete));
					nodeToDelete = -1;
				}
			}
			ImGui::EndPopup();
		}

		if (openLinkPopup)
		{
			ImGui::OpenPopup("NewLink_Popup");
		}

		if (ImGui::BeginPopupModal("NewLink_Popup", nullptr))
		{
			static std::string targetNode = "";
			if (StringInputHeader(mRegistry, "Target State: ", "##newLinkAdd", targetNode))
			{

			}

			if (ImGui::Button("Add"))
			{

				AddLink(nodeToDelete, targetNode);

				targetNode = "";
				nodeToDelete = -1;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}


		
	}

	void AnimatorWindow::DrawTransitionLinkNode(TransitionLinkNode* node)
	{
		ImNodes::Link(node->id, node->source_out_id, node->target_in_id);
		if (ImNodes::IsLinkSelected(node->id))
		{
			if (ImGui::IsWindowFocused())
			{
				SelectLink(static_cast<uint16_t>(node->id));
			
			}
		}
	}

	void AnimatorWindow::CheckTransitionLinkNode()
	{
		int hoveredLinkId = -1;
		static int linkToDelete = -1;

		if (ImNodes::IsLinkHovered(&hoveredLinkId))
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				linkToDelete = hoveredLinkId;
				ImGui::OpenPopup("Link_Popup");
			}
		}
		if (ImGui::BeginPopup("Link_Popup"))
		{
			if (ImGui::Selectable("Delete Link"))
			{
				if (linkToDelete != -1)
				{
					RemoveTransitionFromState(static_cast<uint16_t>(linkToDelete));
					linkToDelete = -1;
				}
			}
			ImGui::EndPopup();
		}
	}

	void AnimatorWindow::LoadDataFromAnimator(SliceEngine::Animator* component, entt::entity entity)
	{
		mCurrentAnimator = component;

		auto guid = mCurrentAnimator->Handle_stateMachine.getGUID();
		mSessionManager->LoadAnimatorData(guid);
		mAnimatorData = mSessionManager->GetAnimatorData();
		
		// Ensure internal editor representation is synced with component once upon loading
		if (mAnimatorData)
			mAnimatorData->LoadFromAsset(mCurrentAnimator->stateMachine.EFSM);

		mSessionManager->currentAnimator = mCurrentAnimator;

		// Set Initial Node Positions
		auto& stateMap = mAnimatorData->mStateMachineAsset->stateMap;

		ImNodes::SetNodeEditorSpacePos(0, mAnimatorData->mStateMachineAsset->entryPosition);
		ImNodes::SetNodeEditorSpacePos(1, mAnimatorData->mStateMachineAsset->exitPosition);

		for (auto& [name, state] : stateMap)
		{
			auto it = mAnimatorData->mNameToStateID.find(name);
			if (it != mAnimatorData->mNameToStateID.end())
			{
				ImNodes::SetNodeEditorSpacePos(it->second, state.mNodePos);
			}
		}
	}

	void AnimatorWindow::ClearData()
	{
		mCurrentAnimator = nullptr;
		mAnimatorData = nullptr;
	}

	void AnimatorWindow::CreateNode(std::string newName)
	{
		mAnimatorData->create_state(newName);

		//auto anim = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Animator>(tmpEnt);
		//LoadDataFromAnimator(anim, tmpEnt);
		//CheckForAnimator();
	}

	void AnimatorWindow::DeleteNode(uint16_t id)
	{
		auto node_it = mAnimatorData->mStateNodes.find(id);

		if (node_it == mAnimatorData->mStateNodes.end())
		{
			SLICE_LOG_ERROR("Node ID not found.");
			return;
		}

		auto stateName = mAnimatorData->mStateNodes.at(id)->name;
		auto& stateMap = mAnimatorData->mStateMachineAsset->stateMap;
		auto state_it = stateMap.find(stateName);

		if (state_it == stateMap.end())
		{
			SLICE_LOG_ERROR("State not found in State Map.");
			return;
		}

		for (auto transistion : stateMap[stateName].transitions)
		{
			RemoveTransitionFromState(transistion.id);
		}

		for (auto state : stateMap)
		{
			for (auto transition : state.second.transitions)
			{
				if (std::strcmp(transition.targetState.c_str(), stateName.c_str()) == 0)
				{
					RemoveTransitionFromState(transition.id);
				}
			}
		}

		stateMap.erase(state_it);
		
		mAnimatorData->mStateNodes.erase(node_it);

		auto param_it = mAnimatorData->mStateMachineAsset->parameters.find(stateName);

		if (param_it == mAnimatorData->mStateMachineAsset->parameters.end())
		{
			SLICE_LOG_ERROR("State not found in Parameters.");
			return;
		}

		mAnimatorData->mStateMachineAsset->parameters.erase(stateName);
	}

	void AnimatorWindow::SelectNode(uint16_t id)
	{
		auto& node = mAnimatorData->mStateNodes.at(id);
		mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(node.get());
	}

	void AnimatorWindow::SelectLink(uint16_t id)
	{
		auto& node = mAnimatorData->mTransitionNodes.at(id);
		mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(node.get());
	}

	void AnimatorWindow::AddLink(uint16_t sourceId,std::string targetState)
	{
		auto& stateMap = mAnimatorData->mStateMachineAsset->stateMap;
		auto state_it = stateMap.find(targetState);
		if (state_it == stateMap.end())
		{
			SLICE_LOG_ERROR("State not found in State Map.");
			return;
		}
		StateNode& sourceNode = *mAnimatorData->mStateNodes.at(sourceId);
		if (std::strcmp(sourceNode.name.c_str(), "Entry") == 0)
		{
			auto node_it = mAnimatorData->mNameToStateID.find(targetState);
			if (node_it == mAnimatorData->mNameToStateID.end())
			{
				SLICE_LOG_ERROR("State not found in animator data.");
				return;
			}

			int targetId = mAnimatorData->mNameToStateID.at(targetState);
			StateNode& targetNode = *mAnimatorData->mStateNodes.at(targetId);
			mAnimatorData->create_link(sourceNode, targetNode);

			mAnimatorData->mStateMachineAsset.get()->entryState = targetState;
		}
		else
		{
			state_it = stateMap.find(sourceNode.name);
			if (state_it == stateMap.end())
			{
				SLICE_LOG_ERROR("State not found in State Map.");
				return;
			}


			auto node_it = mAnimatorData->mNameToStateID.find(targetState);
			if (node_it == mAnimatorData->mNameToStateID.end())
			{
				SLICE_LOG_ERROR("State not found in animator data.");
				return;
			}

			int targetId = mAnimatorData->mNameToStateID.at(targetState);
			StateNode& targetNode = *mAnimatorData->mStateNodes.at(targetId);
			mAnimatorData->create_link(sourceNode, targetNode);

			SliceEngine::SliceEngineTypes::State& sourceState = stateMap.at(sourceNode.name);
			SliceEngine::SliceEngineTypes::Transition tmpTransition{};
			SliceEngine::SliceEngineTypes::Condition tmpCondition{};

			tmpCondition.op = SliceEngine::SliceEngineTypes::ComparisonOp::IsTrue;
			tmpCondition.paramName = targetState;

			tmpTransition.conditions.push_back(tmpCondition);
			tmpTransition.entryTime = 0.0f;
			tmpTransition.hasExitTime = false;
			tmpTransition.exitTime = 1.0f;
			tmpTransition.sourceState = sourceNode.name;
			tmpTransition.targetState = targetState;
			tmpTransition.id = static_cast<int>(mAnimatorData->mTransitionNodes.size()) - 1;
			sourceState.transitions.push_back(tmpTransition);

			std::pair<std::string, rttr::variant> tmpParam{};
			tmpParam.first = targetState;
			tmpParam.second = false;

			mAnimatorData->mStateMachineAsset->parameters.emplace(tmpParam);


			sourceNode.transitionIds.push_back(tmpTransition.id);
		}
	}

	bool AnimatorWindow::RemoveTransitionFromState(uint16_t id)
	{
		auto transition_it = mAnimatorData->mTransitionNodes.find(id);

		if (transition_it == mAnimatorData->mTransitionNodes.end())
		{
			SLICE_LOG_ERROR("Transition ID not found.");
			return false;
		}

		auto& state_node = *mAnimatorData->mStateNodes.at(static_cast<const unsigned short>(transition_it->second->source_id));
		
		auto& stateMap = mAnimatorData->mStateMachineAsset->stateMap;
		if (std::strcmp(state_node.name.c_str(), "Entry") == 0)
		{
			mAnimatorData->mTransitionNodes.erase(transition_it);
			mAnimatorData->mStateMachineAsset.get()->entryState = "";

			return true;
		}
		else
		{
			auto state_it = stateMap.find(state_node.name);

			if (state_it == stateMap.end())
			{
				SLICE_LOG_ERROR("State name: " + state_node.name + " not found in " + mAnimatorData->mStateMachineAsset->assetName + ", transition not removed.");
				return false;
			}

			auto& state = state_it->second;

			auto it = std::find_if(state.transitions.begin(), state.transitions.end(), [&](const auto& transition) { return transition.id == id; });
			if (it == state.transitions.end())
			{
				SLICE_LOG_ERROR("Transition ID not found.");
				return false;
			}

			state.transitions.erase(it);
			auto it2 = std::find(state_node.transitionIds.begin(), state_node.transitionIds.end(), id);
			state_node.transitionIds.erase(it2);

			mAnimatorData->mTransitionNodes.erase(transition_it);

			return true;
		}
	}
}

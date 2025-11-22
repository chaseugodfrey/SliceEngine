#include <pch.h>
#include "AnimatorWindow.h"
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include "Inspector/ComponentPropertiesGUI.h"

namespace SliceEditor
{
	bool AnimatorWindow::CheckStateInput(StateNode* node)
	{
		int id = static_cast<StateNode*>(node)->id;
		if (ImNodes::IsNodeHovered(&id))
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("Node_Popup");
			}
		}
		return ImNodes::IsNodeSelected(id);
	}

	bool AnimatorWindow::CheckLinkInput(TransitionLinkNode* node)
	{
		int id = static_cast<TransitionLinkNode*>(node)->id;
		if (ImNodes::IsLinkHovered(&id))
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("Node_Popup");
			}
		}

		return ImNodes::IsLinkSelected(id);
	}

	void AnimatorWindow::DrawStateNode(StateNode* node)
	{
		ImNodes::BeginNode(node->id);

		//ImNodes::BeginNodeTitleBar();
		//ImGui::TextUnformatted(name.c_str());
		//ImNodes::EndNodeTitleBar();

		ImNodes::BeginInputAttribute(node->in_id);
		ImGui::Text("");
		ImNodes::EndInputAttribute();

		ImGui::SameLine();
		ImGui::TextUnformatted(node->name.c_str());
		ImGui::SameLine();

		ImGui::SameLine();
		ImNodes::BeginOutputAttribute(node->out_id);
		//ImGui::Indent(40);
		ImGui::Text("");
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();
	}

	void AnimatorWindow::DrawTransitionLinkNode(TransitionLinkNode* node)
	{
		ImNodes::Link(node->id, node->source_id, node->target_id);
	}

	AnimatorWindow::~AnimatorWindow()
	{
		if (m_Context)
		{
			NodeEditor::DestroyEditor(m_Context);
			m_Context = nullptr;
		}
	}

	void AnimatorWindow::Init()
	{
	}

	void AnimatorWindow::Draw()
	{
		bool hasAnimator = CheckForAnimator();

		ImGui::Begin("Animator");
#pragma region Animator Toolbar
		ImGui::BeginGroup();
		ImGui::Text("Parameters");
		if (ImGui::BeginMenuBar())
		{
			ImGui::EndMenuBar();
		}

		ImGui::EndGroup();
#pragma endregion

#pragma region Animator Params

		ImGui::BeginChild("##left_region", ImVec2(0.3f * ImGui::GetWindowWidth(), 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);

		if (hasAnimator)
		{
			auto& param_map = mStateMachine->parameters;
			
			int param_id{};
			for (auto& [name, param] : param_map)
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

		ImGui::EndChild();
#pragma endregion
		ImGui::SameLine();
#pragma region Animator Canvas
		
		ImGui::BeginChild("##right_region", ImVec2(0.7f * ImGui::GetWindowWidth(), 0.0f), ImGuiChildFlags_Borders);

		ImNodes::BeginNodeEditor();

		for (auto& [key, node] : mNameToNodeMap)
		{
			DrawStateNode(&node);
		}

		for (auto& link : mLinkList)
		{
			DrawTransitionLinkNode(&link);
		}

		ImNodes::MiniMap();
		ImNodes::EndNodeEditor();
		ImGui::EndChild();
#pragma endregion

		for (auto& [key, node] : mNameToNodeMap)
		{
			if (CheckStateInput(&node))
			{
				mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(&node);
			}
		}

		for (auto& link : mLinkList)
		{
			if (CheckLinkInput(&link))
			{
				mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(&link);
			}
		}

		if (ImNodes::IsEditorHovered())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("NodeEditor_Popup");
			}

		}

		if (ImGui::BeginPopup("Node_Popup"))
		{
			if (ImGui::Selectable("Test"))
			{

			}

			ImGui::EndPopup();
		}

		else if (ImGui::BeginPopup("Node_Popup"))
		{
			if (ImGui::Selectable("Create Node"))
			{

			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	bool AnimatorWindow::CheckForAnimator()
	{
		// Check if any entities selected
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		if (selectionManager->mSelectionType != SelectionType::ENTITY)
		{
			return (mCurrentAnimator != nullptr);
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

			// if anim exists
			if (anim && anim->IsValid())
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

			else
				return (mCurrentAnimator != nullptr);
		}

		// if no entities present
		else
			return (mCurrentAnimator != nullptr);
	}

	void AnimatorWindow::LoadDataFromAnimator(SliceEngine::Animator* component, entt::entity entity)
	{
		mCurrentAnimator = component;

		auto& stateMachineHandle = mCurrentAnimator->Handle_stateMachine;
		mStateMachine = stateMachineHandle.get();

		int nodeId{ 0 };

		for (auto& [name, state] : mStateMachine->stateMap)
		{
			StateNode node{};
			node.id = nodeId;
			node.in_id = nodeId * 2;
			node.out_id = node.in_id + 1;
			node.name = name;
			node.position = ImVec2(state.mNodePos);
			node.state = &state;

			mNameToNodeMap.emplace(name, node);
			//mIndexToNodeMap.emplace(nodeId, node);
			nodeId++;
		}

		int link_id{ 0 };

		for (auto& [name, state] : mStateMachine->stateMap)
		{
			for (auto& transition : state.transitions)
			{
				auto it = mNameToNodeMap.find(transition.targetState);
				if (it == mNameToNodeMap.end())
					continue;

				auto& sourceNode = mNameToNodeMap.at(name);
				auto& targetNode = it->second;
				
				TransitionLinkNode link{};
				link.id = link_id++;
				link.source_id = sourceNode.out_id;
				link.target_id = targetNode.in_id;
				link.transition = &transition;

				mLinkList.push_back(link);
			}
		}
	}

	void AnimatorWindow::ClearData()
	{
		mCurrentAnimator = nullptr;
		mStateMachine = nullptr;
		mNameToNodeMap.clear();
		mLinkList.clear();
	}
}
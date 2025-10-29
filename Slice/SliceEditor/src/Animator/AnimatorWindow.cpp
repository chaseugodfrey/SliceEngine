#include <pch.h>
#include "AnimatorWindow.h"



namespace SliceEditor
{
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
		// To do: save into an editor config path
		NodeEditor::Config config;
		config.SettingsFile = "AnimatorEditor.json";
		m_Context = NodeEditor::CreateEditor(&config);

		// Create some sample nodes
		m_Nodes.push_back({ NodeEditor::NodeId(1), NodeEditor::PinId(2), NodeEditor::PinId(3), "Idle" });
		m_Nodes.push_back({ NodeEditor::NodeId(4), NodeEditor::PinId(5), NodeEditor::PinId(6), "Walk" });
		//m_Nodes.push_back({ NodeEditor::NodeId(3), NodeEditor::PinId(3), "Another" });

	}

	void AnimatorWindow::Draw()
	{

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

#pragma region Animator Canvas
		NodeEditor::SetCurrentEditor(m_Context);
		NodeEditor::Begin("Animator Editor", ImVec2(0.0, 0.0f));

		NodeEditor::EnableShortcuts(true);

		// Draw Nodes here
		for (auto& node : m_Nodes)
		{
			// Draw Nodes here
			NodeEditor::BeginNode(node.Id);
			ImGui::Text(node.Name.c_str());
			NodeEditor::BeginPin(node.outputPinId, NodeEditor::PinKind::Output);
			ImGui::Text("o");
			NodeEditor::EndPin();
			ImGui::SameLine();
			NodeEditor::BeginPin(node.inputPinId, NodeEditor::PinKind::Input);
			ImGui::Text("o");
			NodeEditor::EndPin();
			NodeEditor::EndNode();
		}

		//auto& style = NodeEditor::GetStyle();
		//style.LinkStrength = 1.0f; // reduces curvature toward a straight line
		//
		//NodeEditor::Suspend();

		//NodeEditor::NodeId contextNodeId = 0;
		//NodeEditor::LinkId contextLinkId = 0;

		//if (NodeEditor::ShowNodeContextMenu(&contextNodeId))
		//{
		//	ImGui::OpenPopup("NodeContextMenu");
		//}

		////else if (NodeEditor::ShowLinkContextMenu(&contextLinkId))
		////{
		////	ImGui::OpenPopup("LinkContextMenu");
		////}

		////else if (NodeEditor::ShowBackgroundContextMenu())
		////{
		////	ImGui::OpenPopup("BackgroundContextMenu");
		////}

		////// Popups

		//if (ImGui::BeginPopup("NodeContextMenu"))
		//{
		//	if (ImGui::MenuItem("Delete Node"))
		//	{

		//	}
		//	ImGui::EndPopup();
		//}

		////if (ImGui::BeginPopup("LinkContextMenu"))
		////{
		////	if (ImGui::MenuItem("Delete Link"))
		////	{

		////	}
		////	ImGui::EndPopup();
		////}

		////if (ImGui::BeginPopup("BackgroundContextMenu"))
		////{
		////	if (ImGui::MenuItem("Add Node"))
		////	{

		////	}
		////	ImGui::EndPopup();
		////}

		//NodeEditor::Resume();

		//for (auto& link : m_Links)
		//{
		//	NodeEditor::Link(link.Id, link.sourceId, link.targetId);
		//}

		//if (NodeEditor::BeginCreate())
		//{
		//	NodeEditor::PinId inputPinId, outputPinId;
		//	if (NodeEditor::QueryNewLink(&inputPinId, &outputPinId))
		//	{
		//		if (inputPinId && outputPinId) // both are valid, let's accept link
		//		{
		//			// ed::AcceptNewItem() return true when user release mouse button.
		//			if (NodeEditor::AcceptNewItem())
		//			{
		//				//if (inputPinid == NodeEditor::PinKind::Output)
		//				LinkInfo link{ NodeEditor::LinkId(m_Links.size() + 1), inputPinId, outputPinId };

		//				// Since we accepted new link, lets add one to our list of links.
		//				m_Links.push_back(link);

		//				// Draw new link.
		//				NodeEditor::Link(m_Links.back().Id, m_Links.back().sourceId, m_Links.back().targetId);
		//			}

		//			// You may choose to reject connection between these nodes 
		//			// by calling ed::RejectNewItem(). This will allow editor to give
		//			// visual feedback by changing link thickness and color.
		//		}
		//	}
		//}

		//NodeEditor::EndCreate();

		// End Node Drawing
		NodeEditor::End();
		NodeEditor::SetCurrentEditor(nullptr);

#pragma endregion
		ImGui::End();
	}
}
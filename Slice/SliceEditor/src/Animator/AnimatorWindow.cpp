#include <pch.h>
#include "AnimatorWindow.h"

struct LinkInfo
{
	NodeEditor::LinkId Id;
	NodeEditor::PinId  InputId;
	NodeEditor::PinId  OutputId;
};

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

	}

	void AnimatorWindow::Draw()
	{

		static ImVector<LinkInfo> m_Links;
		int m_NextLinkId = 1;





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
		int nodeId = 1;
		NodeEditor::BeginNode(nodeId++);
		ImGui::Text("Node A");
		NodeEditor::BeginPin(nodeId++, NodeEditor::PinKind::Input);
		ImGui::Text("-> In");
		NodeEditor::EndPin();
		NodeEditor::BeginPin(nodeId++, NodeEditor::PinKind::Output);
		ImGui::Text("Out ->");
		NodeEditor::EndPin();
		NodeEditor::EndNode();

		NodeEditor::BeginNode(nodeId++);
		ImGui::Text("Node B");
		NodeEditor::BeginPin(nodeId++, NodeEditor::PinKind::Input);
		ImGui::Text("-> In");
		NodeEditor::EndPin();
		NodeEditor::BeginPin(nodeId++, NodeEditor::PinKind::Output);
		ImGui::Text("Out ->");
		NodeEditor::EndPin();
		NodeEditor::EndNode();

		// Draw Links
		//NodeEditor::Link(nodeId++, 6, 2);

		for (auto& link : m_Links)
		{
			NodeEditor::Link(link.Id, link.InputId, link.OutputId);
		}

		if (NodeEditor::BeginCreate())
		{
			NodeEditor::PinId inputPinId, outputPinId;
			if (NodeEditor::QueryNewLink(&inputPinId, &outputPinId))
			{
				if (inputPinId && outputPinId) // both are valid, let's accept link
				{
					// ed::AcceptNewItem() return true when user release mouse button.
					if (NodeEditor::AcceptNewItem())
					{
						// Since we accepted new link, lets add one to our list of links.
						m_Links.push_back({ NodeEditor::LinkId(m_NextLinkId++), inputPinId, outputPinId });

						// Draw new link.
						NodeEditor::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
					}

					// You may choose to reject connection between these nodes
					// by calling ed::RejectNewItem(). This will allow editor to give
					// visual feedback by changing link thickness and color.
				}
			}
		}

		NodeEditor::EndCreate();
		
		if (NodeEditor::ShowBackgroundContextMenu())
		{
			ImGui::OpenPopup("BackgroundContextMenu");
		}

		

		if (ImGui::BeginPopup("BackgroundContextMenu"))
		{
			if (ImGui::MenuItem("Add Node"))
			{
				// To do: add node at mouse position
			}
			ImGui::EndPopup();
		}

		// End Node Drawing
		NodeEditor::End();
		NodeEditor::SetCurrentEditor(nullptr);
#pragma endregion
		ImGui::End();
	}
}
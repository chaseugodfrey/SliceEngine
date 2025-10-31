#include <pch.h>
#include "NavigationWindow.h"
#include "Selection/SelectionManager.h"

namespace SliceEditor
{
	void NavigationWindow::Init()
	{
		mCompiler.Init();
	}

	void NavigationWindow::Draw()
	{
		std::vector<Entity> entities_with_renderers{};

		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		ImGui::Begin("Navigation");

		if (selectionManager->mSelectionType == SelectionManager::SelectionType::ENTITY)
		{
			auto& nodes = selectionManager->GetSelectedNodes();
			if (!nodes.empty())
			{
				for (auto& node : nodes)
				{
					EntityNode* entity_node = static_cast<EntityNode*>(node);
					auto renderer = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Renderer>(entity_node->entity);
					if (renderer)
						entities_with_renderers.push_back(entity_node->entity);
				}
			}
		}
		
		std::string obj_handle = std::to_string(entities_with_renderers.size()) + " objects selected.";
		ImGui::Text(obj_handle.c_str());

		if (entities_with_renderers.empty())
			ImGui::BeginDisabled();

		if (ImGui::Button("Bake"))
		{
			
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{

		}

		if (entities_with_renderers.empty())
			ImGui::EndDisabled();

		ImGui::End();
	}
}
/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ComponentMultipleSelection.cpp

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Defines the ComponentMultiSelection file, which wraps multi-selection functions to create support for certain multi-selection use cases.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "ComponentMultipleSelection.h"
#include "Selection/SelectionManager.h"
#include <Scripting/ScriptSystem.h>
#include <Scripting/ScriptObject.h>

namespace SliceEditor
{
	bool StringMultipleSelection(SelectionManager* selectionManager, std::string currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			//Do the difference check (this one is for tags)
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					std::string currentTag = SliceEngine::FactoryInstance.GetGOByEntity(currentEntity).GetTag();

					if (currentTag != currentSelection)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool ComboMultipleSelection(SelectionManager* selectionManager,uint32_t currentSelection, bool isMultiSelection,std::function<uint32_t(Entity)> func)
	{
		if (isMultiSelection)
		{
			//Do the difference check (this one is for tags)
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;

					uint32_t currentuInt32_t = func(currentEntity);

					if (currentuInt32_t != currentSelection)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool GUIDMultipleSelection(SelectionManager* selectionManager, SliceEngine::GUID currentSelection, bool isMultiSelection, std::function<SliceEngine::GUID(Entity)> func)
	{
		if (!func)
		{
			return false;
		}

		if (isMultiSelection)
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;

					SliceEngine::GUID currentGUID = func(currentEntity);

					if (currentGUID != currentSelection)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	std::array<bool,3> Vector3MultipleSelection(SelectionManager* selectionManager, glm::vec3 currentSelection, bool isMultiSelection, std::function<glm::vec3(Entity)> func)
	{
		std::array<bool,3> isSelectionDifferent = std::array<bool, 3>{ false,false,false };
		if (isMultiSelection)
		{
			//Do the difference check (this one is for tags)
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;

					glm::vec3 currentVec3 = func(currentEntity);

					if (currentVec3.x != currentSelection.x)
					{
						isSelectionDifferent[0] = true;
					}

					if (currentVec3.y != currentSelection.y)
					{
						isSelectionDifferent[1] = true;
					}

					if (currentVec3.z != currentSelection.z)
					{
						isSelectionDifferent[2] = true;
					}
				}
			}
		}
		return isSelectionDifferent;
	}

#pragma region Script Variables Difference Check

	std::array<bool, 3> ScriptVector3MultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, glm::vec3 currentSelection, bool isMultiSelection)
	{
		std::array<bool, 3> isSelectionDifferent = std::array<bool, 3>{ false,false,false };
		if (isMultiSelection)
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					//Check for the script component
					if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
					{
						continue;
					}
					//Get the script component and check if same script
					SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
					if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
					{
						continue;
					}
					//Same script so here's the actual difference checker.
					auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
					//Get the value:
					auto currentVal = scriptRef->GetFieldValue<glm::vec3>(scriptVarName);

					//Difference check

					if (currentVal.x != currentSelection.x)
					{
						isSelectionDifferent[0] = true;
					}

					if (currentVal.y != currentSelection.y)
					{
						isSelectionDifferent[1] = true;
					}

					if (currentVal.z != currentSelection.z)
					{
						isSelectionDifferent[2] = true;
					}
				}
			}
		}
		return isSelectionDifferent;
	}

	bool ScriptFloatMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, float currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					//Check for the script component
					if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
					{
						continue;
					}
					//Get the script component and check if same script
					SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
					if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
					{
						continue;
					}
					//Same script so here's the actual difference checker.
					auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
					//Get the value:
					auto currentVal = scriptRef->GetFieldValue<float>(scriptVarName);

					//Difference check
					if (currentVal != currentSelection)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool ScriptIntMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, int currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					//Check for the script component
					if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
					{
						continue;
					}
					//Get the script component and check if same script
					SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
					if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
					{
						continue;
					}
					//Same script so here's the actual difference checker.
					auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
					//Get the value:
					auto currentVal = scriptRef->GetFieldValue<int>(scriptVarName);

					//Difference check
					if (currentVal != currentSelection)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool ScriptBoolMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, bool currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					//Check for the script component
					if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
					{
						continue;
					}
					//Get the script component and check if same script
					SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
					if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
					{
						continue;
					}
					//Same script so here's the actual difference checker.
					auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
					//Get the value:
					auto currentVal = scriptRef->GetFieldValue<bool>(scriptVarName);

					//Difference check
					if (currentVal != currentSelection)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool ScriptStringMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::string currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					//Check for the script component
					if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
					{
						continue;
					}
					//Get the script component and check if same script
					SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
					if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
					{
						continue;
					}
					//Same script so here's the actual difference checker.
					auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
					//Get the value:
					auto currentVal = scriptRef->GetFieldValue<std::string>(scriptVarName);

					//Difference check
					if (currentVal != currentSelection)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool ScriptGameObjMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, SliceEngine::GameObject currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					//Check for the script component
					if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
					{
						continue;
					}
					//Get the script component and check if same script
					SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
					if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
					{
						continue;
					}
					//Same script so here's the actual difference checker.
					auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
					//Get the value:
					auto currentVal = scriptRef->GetFieldValue<SliceEngine::GameObject>(scriptVarName);

					//Difference check
					if (currentVal.GetEntity() != currentSelection.GetEntity())
					{
						return true;
					}
				}
			}
		}
		return false;
	}
#pragma endregion

#pragma region Multi-Setting Fucntions

	void ScriptFloatMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, float currentSelection)
	{
		for (auto selectedNode : selectionManager->GetSelectedNodes())
		{
			if (selectedNode->type == SelectionType::ENTITY)
			{
				Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
				//Check for the script component
				if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
				{
					continue;
				}
				//Get the script component and check if same script
				SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
				if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
				{
					continue;
				}
				//Same script so here's the actual difference checker.
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
				//Get the value:
				auto currentVal = scriptRef->GetFieldValue<float>(scriptVarName);

				if (currentVal != currentSelection)
				{
					scriptRef->SetFieldValue(scriptVarName, currentSelection);
					SliceEngine::gScriptSystem->UpdateScriptComponent(currentEntity);
				}
			}
		}
	}

	void ScriptIntMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, int currentSelection)
	{
		for (auto selectedNode : selectionManager->GetSelectedNodes())
		{
			if (selectedNode->type == SelectionType::ENTITY)
			{
				Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
				//Check for the script component
				if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
				{
					continue;
				}
				//Get the script component and check if same script
				SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
				if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
				{
					continue;
				}
				//Same script so here's the actual difference checker.
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
				//Get the value:
				auto currentVal = scriptRef->GetFieldValue<int>(scriptVarName);

				if (currentVal != currentSelection)
				{
					scriptRef->SetFieldValue(scriptVarName, currentSelection);
					SliceEngine::gScriptSystem->UpdateScriptComponent(currentEntity);
				}
			}
		}
	}

	void ScriptBoolMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, bool currentSelection)
	{
		for (auto selectedNode : selectionManager->GetSelectedNodes())
		{
			if (selectedNode->type == SelectionType::ENTITY)
			{
				Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
				//Check for the script component
				if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
				{
					continue;
				}
				//Get the script component and check if same script
				SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
				if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
				{
					continue;
				}
				//Same script so here's the actual difference checker.
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
				//Get the value:
				auto currentVal = scriptRef->GetFieldValue<bool>(scriptVarName);

				if (currentVal != currentSelection)
				{
					scriptRef->SetFieldValue(scriptVarName, currentSelection);
					SliceEngine::gScriptSystem->UpdateScriptComponent(currentEntity);
				}
			}
		}
	}

	void ScriptStringMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::string currentSelection)
	{
		for (auto selectedNode : selectionManager->GetSelectedNodes())
		{
			if (selectedNode->type == SelectionType::ENTITY)
			{
				Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
				//Check for the script component
				if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
				{
					continue;
				}
				//Get the script component and check if same script
				SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
				if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
				{
					continue;
				}
				//Same script so here's the actual difference checker.
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
				//Get the value:
				auto currentVal = scriptRef->GetFieldValue<std::string>(scriptVarName);

				if (currentVal != currentSelection)
				{
					scriptRef->SetFieldValue(scriptVarName, currentSelection);
					SliceEngine::gScriptSystem->UpdateScriptComponent(currentEntity);
				}
			}
		}
	}

	void ScriptVector3MultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, glm::vec3 currentSelection, std::array<bool,3>& changedAxis)
	{
		for (auto selectedNode : selectionManager->GetSelectedNodes())
		{
			if (selectedNode->type == SelectionType::ENTITY)
			{
				Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
				//Check for the script component
				if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
				{
					continue;
				}
				//Get the script component and check if same script
				SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
				if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
				{
					continue;
				}
				//Same script so here's the actual difference checker.
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
				//Get the value:
				auto currentVal = scriptRef->GetFieldValue<glm::vec3>(scriptVarName);

				//Copy the value to set here:
				glm::vec3 currentValCopy = currentVal;

				if (changedAxis[0])
				{
					currentValCopy.x = currentSelection.x;
				}

				if (changedAxis[1])
				{
					currentValCopy.y = currentSelection.y;
				}

				if (changedAxis[2])
				{
					currentValCopy.z = currentSelection.z;
				}
				if(currentValCopy != currentVal)
				{
					scriptRef->SetFieldValue(scriptVarName, currentValCopy);
					SliceEngine::gScriptSystem->UpdateScriptComponent(currentEntity);
				}
			}
		}
	}

	void ScriptGameObjMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, SliceEngine::GameObject currentSelection)
	{
		for (auto selectedNode : selectionManager->GetSelectedNodes())
		{
			if (selectedNode->type == SelectionType::ENTITY)
			{
				Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
				//Check for the script component
				if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Script>(currentEntity))
				{
					continue;
				}
				//Get the script component and check if same script
				SliceEngine::Script& currentScript = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(currentEntity);
				if (currentScript.scriptName.empty() || currentScript.scriptName != scriptName)
				{
					continue;
				}
				//Same script so here's the actual difference checker.
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(currentEntity);
				//Get the value:
				auto currentVal = scriptRef->GetFieldValue<SliceEngine::GameObject>(scriptVarName);

				if (currentVal.GetEntity() != currentSelection.GetEntity())
				{
					scriptRef->SetFieldValue(scriptVarName, currentSelection);
					SliceEngine::gScriptSystem->UpdateScriptComponent(currentEntity);
				}
			}
		}
	}
#pragma endregion
}
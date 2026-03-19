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

#pragma region Script List Difference Check

	std::vector<bool> ScriptFloatListElementDifferent(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<float> originalList ,bool isMultiSelection)
	{
		std::vector<bool> elementDiffs;
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
					auto currentList = scriptRef->GetListFieldValue<float>(scriptVarName);

					//Compare this list and the "main" list
					size_t maxSize = std::max(currentList.size(), originalList.size());
					if (elementDiffs.size() < maxSize)
					{
						elementDiffs.resize(maxSize, false); //Set everything to false first.
					}

					//Difference check per element
					for (size_t i = 0; i < maxSize; ++i)
					{
						bool currentMissing = i >= currentList.size(); //This means the currentList has less variables than the "main"
						bool originalMissing = i >= originalList.size(); //This means the originalList has less variables than the currently checked one

						if (currentMissing || originalMissing)
						{
							//elementDiffs[i] = true; //Idk do i need to set it as diff??? or just ignore it TBC
							continue;
						}

						if (currentList[i] != originalList[i])
						{
							elementDiffs[i] = true;
						}
					}
				}
			}
		}
		else
		{
			elementDiffs.resize(originalList.size(), false);
		}
		return elementDiffs;
	}

	std::vector<bool> ScriptGameObjectListElementDifferent(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<SliceEngine::GameObject> originalList, bool isMultiSelection)
	{
		std::vector<bool> elementDiffs;
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
					auto currentList = scriptRef->GetListFieldValue<SliceEngine::GameObject>(scriptVarName);

					//Compare this list and the "main" list
					size_t maxSize = std::max(currentList.size(), originalList.size());
					if (elementDiffs.size() < maxSize)
					{
						elementDiffs.resize(maxSize, false); //Set everything to false first.
					}

					//Difference check per element
					for (size_t i = 0; i < maxSize; ++i)
					{
						bool currentMissing = i >= currentList.size(); //This means the currentList has less variables than the "main"
						bool originalMissing = i >= originalList.size(); //This means the originalList has less variables than the currently checked one

						if (currentMissing || originalMissing)
						{
							//elementDiffs[i] = true; //Idk do i need to set it as diff??? or just ignore it TBC
							continue;
						}

						if (currentList[i].GetEntity() != originalList[i].GetEntity())
						{
							elementDiffs[i] = true;
						}
					}
				}
			}
		}
		else
		{
			elementDiffs.resize(originalList.size(), false);
		}
		return elementDiffs;
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

	void ScriptFloatListMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<float> originalList,std::vector<MultiSelect>& changedVars)
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
				auto currentList = scriptRef->GetListFieldValue<float>(scriptVarName);
				//Referring to the same script entity
				if (currentList == originalList)
				{
					continue;
				}

				//Copy the value to set here:
				std::vector<float> currentListCopy = currentList;

				//Check vector sizes.
				size_t maxSize = std::max(currentList.size(), originalList.size());
				size_t safeBounds = std::max(maxSize, changedVars.size());
				
				//Other relevant variables
				float lastValue = 0.0f; //For tracking the last value

				for (size_t i = 0; i < safeBounds; ++i)
				{
					bool currentMissing = i >= currentListCopy.size(); //This means the currentList has less variables than the "main"
					bool originalMissing = i >= originalList.size(); //This means the originalList has less variables than the currently checked one (eg. the main checked has 3 var, but this one has 5)

					if (currentMissing) //Means we added a new variable
					{
						if(changedVars[i] == MultiSelect::ADDED) //This is to make sure removing from a larger "main checked out" does not add to the smaller values. But this now disables multi-addition for smaller lists to larger lists.
						{
							for(size_t j = currentListCopy.size(); j < originalList.size(); ++j) //Add the smaller list until it reaches the size of the originalList
							{
								scriptRef->AddListFieldValue(scriptVarName, lastValue);
							}
						}
						continue;
					}
					else
					{
						lastValue = currentListCopy[i];
					}

					if (originalMissing) //This value doesnt exist on the main (5th variable while the original has only 3
					{
						if (i >= changedVars.size()) //Larger than even the original before removing (only relevant for removing)
						{
							continue;
						}
						/*If you remove 2nd element and the other selected has 5 elements. Only the first changedVars[i] removed will exist. The rest should be removed or not(?)
						With the above if check, it will only remove the ONE element that has been marked for removal.*/
						if (changedVars[i] == MultiSelect::REMOVED)
						{
							if (i == (changedVars.size() - 1))
							{
								for (size_t idx = currentListCopy.size() - 1; idx >= i; --idx)
								{
									scriptRef->RemoveListField(scriptVarName, idx);
								}
								break;
							}
							scriptRef->RemoveListField(scriptVarName, i);
						}
						continue; //Dont do anything (mostly for adding)
					}

					if (changedVars[i] == MultiSelect::CHANGED)
					{
						currentListCopy[i] = originalList[i];
					}

					if (changedVars[i] == MultiSelect::REMOVED)
					{
						scriptRef->RemoveListField(scriptVarName, i);
					}
				}

				if (currentListCopy != currentList)
				{
					scriptRef->SetListField(scriptVarName, currentListCopy);
					SliceEngine::gScriptSystem->UpdateScriptComponent(currentEntity);
				}
			}
		}
	}

	void ScriptGameObjectListMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<SliceEngine::GameObject> originalList, std::vector<MultiSelect>& changedVars)
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
				auto currentList = scriptRef->GetListFieldValue<SliceEngine::GameObject>(scriptVarName);
				//Referring to the same script entity
				if (currentList == originalList)
				{
					continue;
				}

				//Copy the value to set here:
				std::vector<SliceEngine::GameObject> currentListCopy = currentList;

				//Check vector sizes.
				size_t maxSize = std::max(currentList.size(), originalList.size());
				size_t safeBounds = std::max(maxSize, changedVars.size());

				//Other relevant variables
				SliceEngine::GameObject lastValue = SliceEngine::GameObject(); //For tracking the last value

				for (size_t i = 0; i < safeBounds; ++i)
				{
					bool currentMissing = i >= currentListCopy.size(); //This means the currentList has less variables than the "main"
					bool originalMissing = i >= originalList.size(); //This means the originalList has less variables than the currently checked one (eg. the main checked has 3 var, but this one has 5)

					if (currentMissing) //Means we added a new variable
					{
						if (changedVars[i] == MultiSelect::ADDED) //This is to make sure removing from a larger "main checked out" does not add to the smaller values. But this now disables multi-addition for smaller lists to larger lists.
						{
							for (size_t j = currentListCopy.size(); j < originalList.size(); ++j) //Add the smaller list until it reaches the size of the originalList
							{
								scriptRef->AddListFieldValue(scriptVarName, lastValue);
							}
						}
						continue;
					}
					else
					{
						lastValue = currentListCopy[i];
					}

					if (originalMissing) //This value doesnt exist on the main (5th variable while the original has only 3
					{
						if (i >= changedVars.size()) //Larger than even the original before removing (only relevant for removing)
						{
							continue;
						}
						/*If you remove 2nd element and the other selected has 5 elements. Only the first changedVars[i] removed will exist. The rest should be removed or not(?)
						With the above if check, it will only remove the ONE element that has been marked for removal.*/
						if (changedVars[i] == MultiSelect::REMOVED)
						{
							if (i == (changedVars.size() - 1))
							{
								for (size_t idx = currentListCopy.size() - 1; idx >= i; --idx)
								{
									scriptRef->RemoveListField(scriptVarName, idx);
								}
								break;
							}
							scriptRef->RemoveListField(scriptVarName, i);
						}
						continue; //Dont do anything (mostly for adding)
					}

					if (changedVars[i] == MultiSelect::CHANGED)
					{
						currentListCopy[i] = originalList[i];
					}

					if (changedVars[i] == MultiSelect::REMOVED)
					{
						scriptRef->RemoveListField(scriptVarName, i);
					}
				}

				if (currentListCopy != currentList)
				{
					scriptRef->SetListField(scriptVarName, currentListCopy);
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
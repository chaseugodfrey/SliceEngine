/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ComponentMultipleSelection.h

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Declares the ComponentMultiSelection file, which wraps multi-selection functions to create support for certain multi-selection use cases.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef COMPONENT_MULTIPLE_SELECTION_H
#define COMPONENT_MULTIPLE_SELECTION_H

#include <Selection/SelectionManager.h>

namespace SliceEditor
{
	class Registry;
	class SelectionManager;

	enum class MultiSelect
	{
		UNCHANGED,
		CHANGED,
		ADDED,
		REMOVED
	};

	bool StringMultipleSelection(SelectionManager* selectionManager,std::string currentSelection, bool isMultiSelection);

	std::array<bool, 3> ScriptVector3MultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, glm::vec3 currentSelection, bool isMultiSelection);

	bool ScriptFloatMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, float currentSelection, bool isMultiSelection);

	bool ScriptIntMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, int currentSelection, bool isMultiSelection);

	bool ScriptBoolMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, bool currentSelection, bool isMultiSelection);

	bool ScriptStringMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::string currentSelection, bool isMultiSelection);

	bool ScriptGameObjMultipleSelection(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, SliceEngine::GameObject currentSelection, bool isMultiSelection);

	std::vector<bool> ScriptFloatListElementDifferent(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<float> originalList, bool isMultiSelection);

	std::vector<bool> ScriptGameObjectListElementDifferent(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<SliceEngine::GameObject> originalList, bool isMultiSelection);

	bool ComboMultipleSelection(SelectionManager* selectionManager, uint32_t currentSelection, bool isMultiSelection, std::function<uint32_t(Entity)> func);

	bool GUIDMultipleSelection(SelectionManager* selectionManager, SliceEngine::GUID currentSelection, bool isMultiSelection, std::function<SliceEngine::GUID(Entity)> func);

	std::array<bool, 3> Vector3MultipleSelection(SelectionManager* selectionManager, glm::vec3 currentSelection, bool isMultiSelection, std::function<glm::vec3(Entity)> func);

	//std::array<bool, 3> RotationMultipleSelection(SelectionManager* selectionManager, glm::quat currentRot, glm::vec3 currentHint, bool isMultiSelection, std::function<glm::vec3(Entity)> func);

#pragma region Multi-Setting Functions

	void ScriptFloatMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, float currentSelection);

	void ScriptIntMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, int currentSelection);

	void ScriptBoolMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, bool currentSelection);

	void ScriptStringMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::string currentSelection);

	void ScriptVector3MultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, glm::vec3 currentSelection, std::array<bool, 3>& changedAxis);

	void ScriptFloatListMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<float> originalList, std::vector<MultiSelect>& changedVars);

	void ScriptGameObjectListMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, std::vector<SliceEngine::GameObject> originalList, std::vector<MultiSelect>& changedVars);

	void ScriptGameObjMultiSet(SelectionManager* selectionManager, std::string scriptName, std::string scriptVarName, SliceEngine::GameObject currentSelection);

#pragma endregion

}

#endif
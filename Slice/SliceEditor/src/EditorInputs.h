#ifndef EDITOR_INPUTS_H
#define EDITOR_INPUTS_H

namespace SliceEditor
{
	class Registry;

	class EditorInputs
	{
		Registry& registry;

	public:
		bool isInspectorFocused;
		EditorInputs(Registry& reg, bool inspectorFocus) : registry(reg), isInspectorFocused(inspectorFocus) {};
		bool isActive;

		void Init();
		void Update();
	};
}

#endif
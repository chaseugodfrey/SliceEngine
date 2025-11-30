#ifndef EDITOR_INPUTS_H
#define EDITOR_INPUTS_H

namespace SliceEditor
{
	class Registry;

	class EditorInputs
	{
		Registry& registry;

	public:
		EditorInputs(Registry& reg) : registry(reg) {};
		bool isActive;

		void Init();
		void Update();
	};
}

#endif
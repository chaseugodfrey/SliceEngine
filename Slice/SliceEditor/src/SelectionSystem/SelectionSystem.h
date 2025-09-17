#ifndef SELECTION_SYSTEM_H
#define SELECTION_SYSTEM_H

namespace SliceEditor
{
	class SelectionSystem
	{
	public:
		SelectionSystem() = default;
		~SelectionSystem() = default;

	private:
		void UpdateManagers();
	};
}

#endif
#ifndef SELECTION_MANAGER_H
#define SELECTION_MANAGER_H

namespace SliceEditor
{
	class SelectionManager
	{
	public:
		SelectionManager() = default;
		~SelectionManager() = default;

	private:
		void UpdateManagers();
	};
}

#endif
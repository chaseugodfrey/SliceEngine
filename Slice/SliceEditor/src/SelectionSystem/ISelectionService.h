#ifndef INTERFACE_SELECTION_SERVICE_H
#define INTERFACE_SELECTION_SERVICE_H

namespace SliceEditor
{
	class ISelectionListener
	{
	public:
		virtual void OnUpdateSelected(std::unordered_set<entt::entity>&) = 0;
	};
}

#endif
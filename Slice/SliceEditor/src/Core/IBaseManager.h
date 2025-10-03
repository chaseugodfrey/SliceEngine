#ifndef EDITOR_BASE_MANAGER_H
#define EDITOR_BASE_MANAGER_H

namespace SliceEditor
{
	class Registry;

	class IBaseManager
	{

	protected:
		Registry& registry;

	public:
		IBaseManager(Registry& reg) : registry(reg) {};
		virtual ~IBaseManager() = default;
		virtual void Init() = 0;
		Registry& GetRegistry() { return registry; }
	};
}

#endif
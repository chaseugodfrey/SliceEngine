#ifndef COMMANDS_H
#define COMMANDS_H

#include <entt.hpp>

namespace SliceEditor
{
	class SelectionManager;

	class Command
	{
	public:
		virtual void Redo() = 0;
		virtual void Undo() = 0;
		virtual ~Command() = default;
	};

	template<typename T>
	class ValueCommand : public Command
	{
		T& ref, oldValue, newValue;

	public:

		ValueCommand(T& r, T oldV, T newV) : ref(r), oldValue(oldV), newValue(newV) {}
		~ValueCommand() = default;

		void Redo() override
		{
			ref = newValue;
		}

		void Undo() override
		{
			ref = oldValue;
		}
	};

	class SelectEntityCommand : public Command
	{
		SelectionManager& sSelection;
		std::unordered_set<entt::entity> oldEntities;
		std::unordered_set<entt::entity> newEntities;

	public:

		SelectEntityCommand(SelectionManager& sys, std::unordered_set<entt::entity>const& oldE, std::unordered_set<entt::entity>const& newE) :
			sSelection(sys), oldEntities(oldE), newEntities(newE) {}
		~SelectEntityCommand() = default;

		void Redo() override;
		void Undo() override;
	};
}

#endif

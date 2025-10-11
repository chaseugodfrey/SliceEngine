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

	class CreateEntityCommand : public Command
	{
		entt::entity entity;

	public:

		CreateEntityCommand(entt::entity e) : entity(e) {}
		~CreateEntityCommand() = default;

		void Redo() override;
		void Undo() override;
	};

	// to do: implement this with proper way in mind

	class DeleteEntityCommand : public Command
	{
		entt::entity entity;

	public:

		DeleteEntityCommand(entt::entity e) : entity(e) {}
		~DeleteEntityCommand() = default;

		void Redo() override;
		void Undo() override;
	};

	class ParentEntityCommand : public Command
	{
		entt::entity child;
		entt::entity oldParent;
		entt::entity newParent;

	public:

		ParentEntityCommand(entt::entity c, entt::entity oldP, entt::entity newP) : child(c), oldParent(oldP), newParent(newP) {}
		~ParentEntityCommand() = default;

		void Redo() override;
		void Undo() override;
	};

	template <typename T>
	class AddComponentCommand : public Command
	{
		entt::entity entity;

	public:

		AddComponentCommand(entt::entity e) : entity(e) {}
		~AddComponentCommand() = default;

		void Redo() override
		{
			SliceEngine::Core::GetRegistry()->emplace<T>(entity);
		}

		void Undo() override
		{
			SliceEngine::Core::GetRegistry()->remove<T>(entity);
		}
	};

	// to do: implement this with proper way in mind

	class RemoveComponentCommand : public Command
	{
		entt::entity entity;

	public:

		RemoveComponentCommand(entt::entity e) : entity(e) {}
		~RemoveComponentCommand() = default;

		void Redo() override;
		void Undo() override;
	};
}

#endif

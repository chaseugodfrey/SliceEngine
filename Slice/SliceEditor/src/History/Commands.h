#ifndef COMMANDS_H
#define COMMANDS_H



namespace SliceEditor
{
	class SelectionManager;

	class Command
	{
	protected:
		std::string message;

	public:
		virtual void Redo() = 0;
		virtual void Undo() = 0;
		virtual ~Command() = default;
		std::string const GetCommandMessage();
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
			message = std::string("Changed value to ") + EditorUtilities::ValueToString(oldValue);
			ref = newValue;

		}

		void Undo() override
		{
			message = std::string("Changed value to ") + EditorUtilities::ValueToString(oldValue);
			ref = oldValue;
		}
	};


	template<typename T>
	class ValueCommand<SliceEngine::Handle<T>> : public Command
	{
		SliceEngine::Handle<T>& ref, oldValue, newValue;

	public:

		ValueCommand(SliceEngine::Handle<T>& r, SliceEngine::Handle<T> oldV, SliceEngine::Handle<T> newV) : ref(r), oldValue(oldV), newValue(newV) {}
		~ValueCommand() = default;

		void Redo() override
		{
			message = std::string("Changed value to ") + EditorUtilities::ValueToString(oldValue);
			ref = newValue;

		}

		void Undo() override
		{
			message = std::string("Changed value to ") + EditorUtilities::ValueToString(oldValue);
			ref = oldValue;
		}
	};

	template<typename T>
	class FunctionSetsValueCommand : public Command
	{
		T oldValue, newValue;
		std::function<void(T)> funcToExecute;

	public:

		FunctionSetsValueCommand(T oldV, T newV, std::function<void(T)> func) 
			: oldValue(oldV), newValue(newV), funcToExecute(func){}
		~FunctionSetsValueCommand() = default;

		void Redo() override
		{
			funcToExecute(newValue);
		}

		void Undo() override
		{
			funcToExecute(oldValue);
		}
	};

	template <typename T>
	class ScriptFieldSetterCommand : public Command
	{
		std::function<void(std::string, T)> funcToExecute;
		std::string fieldName;
		T oldValue, newValue;

	public:
		ScriptFieldSetterCommand(std::function<void(std::string, T)>func,  std::string name, T oldV, T newV) :
			funcToExecute(func), fieldName(name), oldValue(oldV), newValue(newV) {}
		~ScriptFieldSetterCommand() = default;

		void Redo() override
		{
			funcToExecute(fieldName, newValue);
		}

		void Undo() override
		{
			funcToExecute(fieldName, oldValue);
		}
	};

	template <typename T>
	class ScriptListSetterCommand : public Command
	{
		std::function<void(const char*, std::string, std::vector<T>, T, int)> funcToExecute;
		std::string fieldName;
		const char* funcType;
		std::vector<T> oldList, newList;
		int index;

	public:
		ScriptListSetterCommand(std::function<void(const char*, std::string, std::vector<T>, T, int)>func, const char* type, std::string name, std::vector<T> oldL, std::vector<T> newL, int idx = 0) :
			funcToExecute(func), funcType(type), fieldName(name), oldList(oldL), newList(newL),index(idx) {
		}
		~ScriptListSetterCommand() = default;

		void Redo() override
		{
			funcToExecute(funcType, fieldName, newList, newList[index], index);
		}

		void Undo() override
		{
			funcToExecute(funcType, fieldName, oldList, oldList[index], index);
		}
	};

	class SelectNodeCommand : public Command
	{
		SelectionManager& sSelection;
		std::unordered_set<SelectionNode*> oldNodes;
		std::unordered_set<SelectionNode*> newNodes;

		std::string ConvertSelectionTypeToString(std::unordered_set<SelectionNode*> const & nodes)
		{
			SelectionType type = SelectionType::NONE;
			bool first = true;
			for (auto& node : nodes)
			{
				if (!node)
				{
					continue;
				}

				if (first)
					type = node->type;
				else
					if (type != node->type)
					{
						type = SelectionType::MIXED;
						break;
					}

				first = false;
			}

			if (!mSelectionTypeToString.contains(type))
			{
				return mSelectionTypeToString.at(SelectionType::NONE);
			}

			return mSelectionTypeToString.at(type);
		}

	public:
		SelectNodeCommand(SelectionManager& sys, std::unordered_set<SelectionNode*> oldN, std::unordered_set<SelectionNode*> newN) :
			sSelection(sys), oldNodes(oldN), newNodes(newN) 
		{

			message = "Selected " + ConvertSelectionTypeToString(newNodes);
		}
		~SelectNodeCommand() = default;
		void Redo() override;
		void Undo() override;
	};

	class SelectEntityCommand : public Command
	{
		SelectionManager& sSelection;
		std::unordered_set<entt::entity> oldEntities;
		std::unordered_set<entt::entity> newEntities;

	public:

		SelectEntityCommand(SelectionManager& sys, std::unordered_set<entt::entity>const& oldE, std::unordered_set<entt::entity>const& newE) :
			sSelection(sys), oldEntities(oldE), newEntities(newE) 
		{
			message = "Selected Entity ";
		}
		~SelectEntityCommand() = default;

		void Redo() override;
		void Undo() override;
	};

	class CreateEntityCommand : public Command
	{
		entt::entity entity;

	public:

		CreateEntityCommand(entt::entity e) : entity(e) {};
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

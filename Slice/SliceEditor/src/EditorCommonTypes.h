#ifndef EDITOR_COMMON_TYPES_H
#define EDITOR_COMMON_TYPES_H

namespace SliceEditor
{
	struct ICommand
	{
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual ~ICommand() = default;
	};
}

#endif
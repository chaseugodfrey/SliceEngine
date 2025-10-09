#include <pch.h>
#include "Commands.h"
#include "Selection/SelectionManager.h"

namespace SliceEditor
{

	void SelectEntityCommand::Undo()
	{
		sSelection.SelectMultiple(oldEntities, true);
	}

	void SelectEntityCommand::Redo()
	{
		sSelection.SelectMultiple(newEntities, true);
	}
}
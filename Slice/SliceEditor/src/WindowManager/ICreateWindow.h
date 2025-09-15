#ifndef INTERFACE_CREATE_WINDOW_H
#define INTERFACE_CREATE_WINDOW_H

#include "EditorWindow.h"

namespace SliceEditor
{
	class ICreateWindow
	{
	public:
		virtual ~ICreateWindow() = default;
		virtual std::unique_ptr<EditorWindow> CreateWindow() = 0;
	};
}
#endif
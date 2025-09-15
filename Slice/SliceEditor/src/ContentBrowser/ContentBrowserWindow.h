#ifndef COnTENT_BROWSER_WINDOW_H
#define CONTENT_BROWSER_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class ContentBrowserWindow : public EditorWindow
	{
	public:
		ContentBrowserWindow() = default;
		~ContentBrowserWindow() = default;
		void Draw() override final;
	};
}

#endif
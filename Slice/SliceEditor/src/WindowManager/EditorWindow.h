#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

namespace SliceEditor
{
	class EditorWindow
	{
	public:
		virtual void Draw() = 0;
		virtual ~EditorWindow() = default;
	};
}

#endif

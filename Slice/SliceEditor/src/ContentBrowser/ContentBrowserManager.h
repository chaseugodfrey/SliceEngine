#ifndef CONTENT_BROWSER_MANAGER_H
#define CONTENT_BROWSER_MANAGER_H

#include <memory>
#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	struct TreeNode;

	class ContentBrowserManager : public ICreateWindow
	{

		std::unique_ptr<TreeNode> rootNode;
		void BuildTree();
	
	public:

		void Init();
		std::unique_ptr<EditorWindow> CreateWindow() override;
	};
}

#endif
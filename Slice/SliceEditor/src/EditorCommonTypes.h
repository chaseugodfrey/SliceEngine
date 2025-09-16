#ifndef EDITOR_COMMON_TYPES_H
#define EDITOR_COMMON_TYPES_H

namespace SliceEditor
{
	struct TreeNode
	{
		std::string name;
		TreeNode* parent;
		TreeNode* child;
		TreeNode* previous;
		TreeNode* next;
	};

	struct DirectoryNode
	{
		std::string fileName;
		bool isDirectory = false;
		std::filesystem::path path;
		DirectoryNode* parent = nullptr;
		std::map<std::string, DirectoryNode> children;
	};

	struct Command
	{
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual ~Command() = default;
	};

	struct SelectionCommand : Command
	{
		TreeNode* previous, *current;
		SelectionCommand(TreeNode* prev, TreeNode* curr) : previous(prev), current(curr) {}
		void Execute() override;
		void Undo() override;
	};
}

#endif
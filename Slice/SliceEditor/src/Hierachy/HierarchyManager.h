#ifndef HIERARCHY_MANAGER_H
#define HIERARCHY_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	struct TestNode
	{
		TestNode* parent;
		entt::entity entity;
		std::string name;
		bool isSelected;
		std::vector<entt::entity> children;

		bool operator==(TestNode& rhs)
		{
			return entity == rhs.entity;
		}
	};

	class Registry;

	class HierarchyManager : public IBaseManager, public ICreateWindow
	{
		// TO DO: replace this with proper scene graph
		std::vector<TestNode> mRootNodes;
		
		std::map<entt::entity, TestNode> mHierarchy;

		bool isDirty;

		void BuildHierarchy();

	public:

		HierarchyManager(Registry& reg) : IBaseManager(reg) {};
		~HierarchyManager() = default;

		void Test();
		void Init() override;
		void SetDirty();
		void CheckDirty();


		void AddGameObject();
		void ParentGameObject(entt::entity child, entt::entity parent);
		void RemoveGameObject(entt::entity target);

		std::unique_ptr<EditorWindow> CreateWindow() override;

		std::vector<TestNode>& GetNodes() { return mRootNodes; }
		std::map<entt::entity, TestNode>& GetHierarchy() { return mHierarchy; }
	};
}

#endif
/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GOFactory.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles things related to GameObjects

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef GO_FACTORY_H
#define GO_FACTORY_H
#include "ECSTypes.h"
#include "GameObject.h"
#include <entt.hpp>
#include <rttr/variant.h>

#include "Resource/Resource.h"

namespace SliceEngine
{
	using ComponentCloner = std::function<void(Registry& reg, Entity eToClone, Entity eToCreate)>;
	using ComponentGetter = std::function <rttr::variant(Registry& reg, Entity e)>;
	using ComponentVisitor = std::function<void(rttr::type, rttr::variant&)>;
	using ComponentEmplacer = std::function<void(Registry&, Entity, const rttr::variant&)>;


	//using EnttIdToRttrType = std::function<rttr::type(entt::id_type type)>;
	//using GetterMapper = std::function<rttr::instance(entt::registry&, entt::entity)>;
	//using InstanceGetter = std::unordered_map<entt::id_type, GetterMapper>;

	//static EnttIdToRttrType EnttIdToRttrTypeFunc;
	//static InstanceGetter InstanceGetterFunc;

	class GOFactory
	{
	public:
		GOFactory();
		~GOFactory();

		// uses entt's emplace_or_replace to clone components
		// this uses component's copy/move constructor
		// so any components that has any pointers or handles, will need a custom clone function
		// but I'm not doing that now
		// if yall need it then lmk
		template<class T>
		void CreateComponentCloner()
		{
			const entt::id_type id = entt::type_id<T>().hash();
			mComponentCloners.emplace(id, [](Registry& reg, Entity toClone, Entity toCreate)
				{
					if (auto* component = reg.try_get<T>(toClone))
					{
						if constexpr (std::is_empty_v<T>)
						{
							reg.emplace_or_replace<T>(toCreate);
						}
						else
						{
							reg.emplace_or_replace<T>(toCreate, *component);
						}
					}	
				});
		};

		template<typename Component>
		void RegisterSerializableComponent()
		{
			entt::id_type type_id = entt::type_id<Component>().hash();
			mComponentGetters[type_id] = [](Registry& registry, Entity e) -> rttr::variant {
				
				auto component = registry.try_get<Component>(e);
				if (component) return *component;
				
				return rttr::variant();
				};
		}

		template<typename Component>
		void RegisterComponentEmplacer()
		{
			rttr::type type = rttr::type::get<Component>();
			std::string typeName = type.get_name().to_string();

			//mComponentEmplacer[type] = [](Registry& reg, Entity entity, const rttr::variant& var)
			//	{
			//		// convert variant to our component type 
			//		bool converted;
			//		Component component = var.convert<Component>(&converted);

			//		if (converted)
			//		{
			//			reg.emplace<Component>(entity, component);
			//			SLICE_LOG_DEBUG("Successfully emplaced new component");
			//		}
			//		else
			//		{
			//			SLICE_LOG_ERROR("Unable to register component emplacer");
			//		}
			//	};
		
			// this is so fking stupid
			// cause when getting type of an rttr variant
			// it returns a shared ptr type
			// so the above component emplacer's rttr type is different than the other rttr type
			// why is there different rttr types 
			rttr::type smartPtrType = rttr::type::get<std::shared_ptr<Component>>();
			typeName = smartPtrType.get_name().to_string();

			mCESmartPtr[smartPtrType] = [](Registry& reg, Entity entity, const rttr::variant& var)
			{
				// convert variant to our component type 
				//bool converted;
				//Component component = var.convert<Component>(&converted);
				auto componentPtr = var.convert<std::shared_ptr<Component>>();
				rttr::type type = rttr::type::get<Component>();
				std::string typeName = type.get_name().to_string();

				if (componentPtr)
				{
					reg.emplace_or_replace<Component>(entity, *componentPtr);
					//std::string msg = "Successfully emplaced new component (smart ptr): " + typeName;
					//SLICE_LOG_VALUES("Successfuly emplaced new component: " + typeName);
				}
				else
				{
					SLICE_LOG_ERROR("Unable to register component emplacer");
				}
			};
		}

		template<typename Component>
		void RegisterComponent()
		{
			rttr::type componentType = rttr::type::get<Component>();
			if (!componentType.is_valid())
			{
				SLICE_LOG_ERROR("Component is not registered with RTTR.");
				return;
			}

			RegisterSerializableComponent<Component>();
			RegisterComponentEmplacer<Component>();
			CreateComponentCloner<Component>();

			entt::id_type type_id = entt::type_id<Component>().hash();
			mComponentNames[type_id] = rttr::type::get<Component>().get_name().to_string();
		}

		GameObject CreateBlank(); // for deserializing
		GameObject CreateBlanker(); // for prefab editing. doesn't add to mNameToEntity
		GameObject CreateEO();
		GameObject CreateGO(std::string name = "GameObject");
		GameObject CreateUIGO(std::string name = "UI_GameObject");
		GameObject CloneGO(GameObject const& go);
		GameObject CloneGO(GameObject const& go, Entity parentEntity);
		GameObject GetGOByEntity(Entity entity);
		Entity GetEntityWithTag(std::string const& tag);
		GameObject GetGOByName(std::string name);
		std::vector<Entity> GetEntitiesWithTag(std::string const& tag);
		Entity GetRootEntity();
		void UpdateName(std::string newName, Entity entity);
		void Destroy(GameObject& go);
		void Destroy(entt::entity entity);
		void TestLoop();
		void UpdateDestroyed();
		void SceneGraphDelete(Entity entity);
		void VisitComponents(Entity entity, ComponentVisitor visitor);
		void EmplaceComponents(Entity entity, const rttr::variant& componentVariant);
		std::string CreateName(std::string name);
		void InitRootEntity();
		void RemoveFromNameMap(Entity entity);
		void AddToNameMap(Entity entity);
		void RemoveFromNameMap(std::string name);

		// THESE ARE FOR TESTING
		// @GIDEON RMB TO DELETE OR ANYONE THAT READS THIS
		inline std::string GetNameFromMap(Entity entity)
		{
			for (auto& [name, ent] : mNameToEntity)
			{
				if (ent == entity)
				{
					return name;
				}
			}

			return "";
		}
		inline void PrintNameMap()
		{
			for(auto& [name, entity] : mNameToEntity)
			{
				SLICE_LOG_VALUES("NameMap Entry: " + name);
			}
		}
		bool isDescendant(Entity target, Entity dest);
		bool Unparent(Entity entity);
		bool SetParent(Entity entity, Entity parentEntity = entt::null);
		void SetNewSceneGraphLocation(Entity targetEntity, Entity leftEntity, Entity rightEntity, Entity parentEntity);
		void BuildSceneGraph(std::unordered_map<uint32_t, uint32_t> map);
		void ClearGameObjects();
		void UpdateTransformFromParent(Entity entity, Entity parent, Entity oldParent = entt::null);

		bool CheckValidName(Entity entity);

		void FactoryShutdown();

		// todo : bring to prefab factory
		GameObject CreateGO_Box();
		GameObject CreateGO_Sphere();
		GameObject CreateGO_Capsule();
		GameObject CreateGO_Cylinder();
		GameObject CreateGO_Cam();
		GameObject CreateGO_Light();
		GameObject CreateGO_Model(GUID skele_guid = GUID::null(), GUID anim_guid = GUID::null(), GUID model_guid = GUID(DefaultResourceIDs::CUBE_DEFAULT));
		GameObject CreateGO_Canvas();
		GameObject CreateGO_Image();
		GameObject CreateGO_Button();
		GameObject CreateGO_Slider();
		GameObject CreateGO_Text();

		Registry mRegistry;

		// NOTE: I'm putting this in public for now to test serialization.
		std::unordered_map<entt::id_type, ComponentGetter> mComponentGetters;
		// ngl idk if these maps should be public or private
		// but like editor needs it 
		std::unordered_map<entt::id_type, std::string> mComponentNames;
		void DebugPrint();

		std::unordered_map<entt::id_type, ComponentCloner> mComponentCloners;

	private:
		GameObject CreateGO_ModelNode(SliceEngineTypes::ModelNode const& node, GUID skele_guid, GUID anim_guid, GUID model_node, Entity parent, Entity root, int& index, bool is_static);


		std::unordered_map<std::string, Entity> mNameToEntity;
		std::unordered_map<Entity, GameObject> mEntityToGO;		
		// I really dont like how this emplacing is being done imo(i agree)
		std::unordered_map<rttr::type, ComponentEmplacer> mCESmartPtr;
		std::unordered_map<rttr::type, ComponentEmplacer> mComponentEmplacer;

		std::vector<GameObject> mEngineEntities;
		std::set<Entity> mDeleteList;
		Entity mRootEntity;
	};
}

#endif
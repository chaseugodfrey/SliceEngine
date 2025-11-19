#include <pch.h>
#include "LayerManager.h"
#include "Core/Core.h"
#include "../Physics/PhysicsSystem.h"

namespace SliceEngine
{
	void LayerManager::Init()
	{
		// intialize or deserialie any layer stuff here

		// jus 3 for now
		AddLayer("Default");
		AddLayer("Ignore Raycast");
		AddLayer("UI");
		AddLayer("Player");
		AddLayer("Environment");
		AddLayer("Enemies");
	}

	void LayerManager::AddLayer(std::string name)
	{
		if (currentBit > MAX_LAYERS)
		{
			SLICE_LOG_ERROR("wtf why are u trying to make more than 32 layers??");
			return;
		}

		// check if it alr exist
		if (collisionMask.find(name) != collisionMask.end())
		{
			SLICE_LOG_ERROR(name + " already exist bodoh");
			return;
		}
		uint32_t layerBit{};

		if (removedBits.size() > 0)
		{
			// use the latest bit to be removed
			uint32_t bit = removedBits.back();
			layerBit = 1 << bit;

			collisionMask[name] = layerBit;
			indexToLayerName[currentBit] = name;
			nameToLayer[name] = bit;

			removedBits.pop_back();
		}
		else
		{
			// get the bit for this layer
			layerBit = 1u << currentBit;

			// update both map and vector
			collisionMask[name] = layerBit;
			indexToLayerName[currentBit] = name;
			nameToLayer[name] = currentBit;

			//collisionKeys.push_back(name);

			// increment to next bit
			currentBit++;
		}

		numberOflayers++;

		// Jolt bodies need to know about the new layer and its mask
		Core::GetInstance()->GetSystem<PhysicsSystem>().SetCollisionMask(currentBit, layerBit);

	}

	//Removes the layer from the map and sets entities using it to default layer
	void LayerManager::RemoveLayer(std::string name)
	{
		// check if it doesn't exist
		if (collisionMask.find(name) == collisionMask.end())
		{
			SLICE_LOG_ERROR(name + " doesn't exist bodoh");
			return;
		}

		// Update any entities that were using this layer
		auto view = Core::GetInstance()->GetRegistry().view<Transform>();

		for (auto entity : view)
		{
			SetDefaultLayer(name, entity); // change to default but remove the layer
		}

		// inform jolt that this layer is gone so set its mask to 0
		Core::GetInstance()->GetSystem<PhysicsSystem>().SetCollisionMask(nameToLayer[name], 0);

		// erase from both collision layer and keys
		collisionMask.erase(name);
		nameToLayer.erase(name);

		// i realise since i changed to map from vector, i dont have to loop like this
		// but it works
		uint32_t i = 0;
		for (auto it = indexToLayerName.begin(); it != indexToLayerName.end(); ++it)
		{
			if (it->second == name)
			{
				// if this is the bit that is deleted then push to removedBits
				// so we'll use this for the next layer created
				removedBits.push_back(i);

				// erase from index to layer
				indexToLayerName.erase(i);
				break;
			}

			i++;
		}

		numberOflayers--;
	}

	uint32_t LayerManager::GetMask(std::string name)
	{
		// check if it doesn't exist
		if (collisionMask.find(name) == collisionMask.end())
		{
			SLICE_LOG_ERROR(name + " doesn't exist bodoh");
			return 0;
		}
		
		return collisionMask[name];
	}

	// might not even be using this mayb
	uint32_t LayerManager::GetMask(uint32_t index)
	{
		if (index >= currentBit)
		{
			SLICE_LOG_ERROR(index + " doesn't exist bodoh");
			return 0;
		}

		// doesn't exist
		if (indexToLayerName.find(index) == indexToLayerName.end())
			return 0;

		// looks kinda cancer idk
		return collisionMask[indexToLayerName[index]];
	}

	uint32_t LayerManager::GetLayer(std::string name)
	{
		// check if it doesn't exist
		if (nameToLayer.find(name) == nameToLayer.end())
		{
			SLICE_LOG_ERROR(name + " doesn't exist bodoh");
			return INVALID_LAYER; // invalid layer (max 32 layers)
		}
		else if (nameToLayer[name] < 0u || nameToLayer[name] >= MAX_LAYERS)
		{
			SLICE_LOG_ERROR(name + " layer value is invalid bodoh");
			return INVALID_LAYER; // invalid layer (max 32 layers)
		}

		return nameToLayer[name];
	}

	uint32_t LayerManager::GetLayer(uint32_t index)
	{
		if (index >= currentBit)
		{
			SLICE_LOG_ERROR(index + " doesn't exist bodoh");
			return INVALID_LAYER; // invalid layer (max 32 layers)
		}

		// doesn't exist
		if (indexToLayerName.find(index) == indexToLayerName.end())
			return INVALID_LAYER;

		if (nameToLayer[indexToLayerName[index]] < 0u || nameToLayer[indexToLayerName[index]] >= MAX_LAYERS)
		{
			SLICE_LOG_ERROR(index + " layer value is invalid bodoh");
			return INVALID_LAYER; // invalid layer (max 32 layers)
		}

		// looks kinda cancer idk
		return nameToLayer[indexToLayerName[index]];
	}

	std::string LayerManager::GetLayerName(uint32_t layer)
	{
		if (indexToLayerName.find(layer) == indexToLayerName.end())
		return std::string("no layer bodoh");

		return indexToLayerName[layer];
	}

	uint32_t LayerManager::GetNumberOfLayers() const
	{
		return numberOflayers;
	}

	bool LayerManager::CheckEntityCanInteract(Entity first, Entity second)
	{
		auto entityFirst = FactoryInstance.GetGOByEntity(first);
		auto entitySecond = FactoryInstance.GetGOByEntity(second);

		// if both have transform component
		if (entityFirst.HasComponent<Transform>() && entitySecond.HasComponent<Transform>())
		{
			Transform& firstTransform = entityFirst.GetComponent<Transform>();
			Transform& secondTransform = entitySecond.GetComponent<Transform>();
			if(firstTransform.collisionLayer == INVALID_LAYER || secondTransform.collisionLayer == INVALID_LAYER)
			{
				return false;
			}

			std::string firstLayerName = GetLayerName(firstTransform.collisionLayer);
			std::string secondLayerName = GetLayerName(secondTransform.collisionLayer);

			if(firstLayerName == "no layer bodoh" || secondLayerName == "no layer bodoh")
			{
				SLICE_LOG_ERROR("cant interact bodoh one of these entityFirst or entitySecond has no layer");
				return false;
			}

			// as long as its not 0, means they share a layer
			return collisionMask[firstLayerName] & collisionMask[secondLayerName];
		}

		// if either one of them or both dont have transform component
		// then itll jus return false
		return false;
	}
 
	bool LayerManager::CheckLayerInteraction(std::string first, std::string second)
	{
		uint32_t firstMask = GetMask(first);
		uint32_t secondLayer =GetLayer(second);

		return (firstMask & (1u << secondLayer)) != 0u;

	}

	void LayerManager::AssignLayer(std::string name, Entity entity)
	{
		auto entityGO = FactoryInstance.GetGOByEntity(entity);

		if (entityGO.HasComponent<Transform>())
		{
			auto& transform = entityGO.GetComponent<Transform>();
			transform.collisionLayer = nameToLayer[name];
		}

		//check if it is a physics body to update jolt body layer
		if (entityGO.HasComponent<ColliderShape>())
		{
			Core::GetInstance()->GetSystem<PhysicsSystem>().SetBodyLayer(entity, nameToLayer[name]);
		}
	}

	//rework change all to default layer instead of removing entirely
	void LayerManager::SetDefaultLayer(std::string name, Entity entity)
	{
		auto entityGO = FactoryInstance.GetGOByEntity(entity);

		// idk where we want to store it's collision mask but for now its in Transform
		if (entityGO.HasComponent<Transform>())
		{
			auto& transform = entityGO.GetComponent<Transform>();

			if (GetLayer("Default") == INVALID_LAYER)
			{
				SLICE_LOG_ERROR("Default layer doesn't exist bodoh");
			}
			else
			{
				transform.collisionLayer = GetLayer("Default");
			}
		
		}

		//check if it is a physics body to update jolt body layer
		if (entityGO.HasComponent<ColliderShape>())
		{
			Core::GetInstance()->GetSystem<PhysicsSystem>().SetBodyLayer(entity, nameToLayer[name]);
		}

	}

	void LayerManager::AssignLayerInteraction(std::string first, std::string second, bool canInteract)
	{
		if (canInteract)
		{
			collisionMask[first] |= (1u << nameToLayer[second]);
			collisionMask[second] |= (1u << nameToLayer[first]);
		}
		else
		{
			collisionMask[first] &= ~(1u << nameToLayer[second]);
			collisionMask[second] &= ~(1u << nameToLayer[first]);
		}

		Core::GetInstance()->GetSystem<PhysicsSystem>().SetCollisionMask(nameToLayer[first], collisionMask[first]);
		Core::GetInstance()->GetSystem<PhysicsSystem>().SetCollisionMask(nameToLayer[second], collisionMask[second]);

	}
}


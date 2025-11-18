#include <pch.h>
#include "LayerManager.h"
#include "Core/Core.h"

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

		if (removedBits.size() > 0)
		{
			// use the latest bit to be removed
			uint32_t bit = removedBits.back();
			uint32_t layerBit = 1 << bit;

			collisionMask[name] = layerBit;
			indexToLayerName[currentBit] = name;
			nameToLayer[name] = bit;

			removedBits.pop_back();
		}
		else
		{
			// get the bit for this layer
			uint32_t layerBit = 1 << currentBit;

			// update both map and vector
			collisionMask[name] = layerBit;
			indexToLayerName[currentBit] = name;
			nameToLayer[name] = currentBit;

			//collisionKeys.push_back(name);

			// increment to next bit
			currentBit++;
		}

	}

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
			UnassignLayer(name, entity);
		}


		// erase from both collision layer and keys
		collisionMask.erase(name);

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

		return nameToLayer[name];
	}

	// might not even be using this mayb
	uint32_t LayerManager::GetLayer(uint32_t index)
	{
		if (index >= currentBit)
		{
			SLICE_LOG_ERROR(index + " doesn't exist bodoh");
			return INVALID_LAYER; // invalid layer (max 32 layers)
		}

		// doesn't exist
		if (indexToLayerName.find(index) == indexToLayerName.end())
			return 0;

		// looks kinda cancer idk
		return nameToLayer[indexToLayerName[index]];
	}



	bool LayerManager::CheckLayerInteraction(Entity first, Entity second)
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

			// as long as its not 0, means they share a layer
			return firstTransform.collisionMask & secondTransform.collisionMask;
		}

		// if either one of them or both dont have transform component
		// then itll jus return false
		return false;
	}

	void LayerManager::AssignLayer(std::string name, Entity entity)
	{
		auto entityGO = FactoryInstance.GetGOByEntity(entity);

		// idk where we want to store it's collision mask but for now its in Transform
		if (entityGO.HasComponent<Transform>())
		{
			auto& transform = entityGO.GetComponent<Transform>();

			transform.collisionMask |= collisionMask[name];
			transform.collisionLayer = nameToLayer[name];
		}
	}

	void LayerManager::UnassignLayer(std::string name, Entity entity)
	{
		auto entityGO = FactoryInstance.GetGOByEntity(entity);

		// idk where we want to store it's collision mask but for now its in Transform
		if (entityGO.HasComponent<Transform>())
		{
			auto& transform = entityGO.GetComponent<Transform>();

			// If it has this collision layer
			if (transform.collisionMask & collisionMask[name])
			{
				// remove it
				transform.collisionMask &= ~collisionMask[name];
				transform.collisionLayer = INVALID_LAYER; // set to invalid layer(Max 32 layers)
			}
		}

	}
}


#include <pch.h>
#include "LayerManager.h"
#include "Core/Core.h"

namespace SliceEngine
{
	void LayerManager::Init()
	{
		// intialize or deserialie any layer stuff here

		// jus 3 for now
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
		if (collisionLayers.find(name) != collisionLayers.end())
		{
			SLICE_LOG_ERROR(name + " already exist bodoh");
			return;
		}

		if (removedBits.size() > 0)
		{
			// use the latest bit to be removed
			int bit = removedBits.back();
			uint32_t layerBit = 1 << bit;

			collisionLayers[name] = layerBit;
			indexToLayer[currentBit] = name;

			removedBits.pop_back();
		}
		else
		{
			// get the bit for this layer
			uint32_t layerBit = 1 << currentBit;

			// update both map and vector
			collisionLayers[name] = layerBit;
			indexToLayer[currentBit] = name;

			//collisionKeys.push_back(name);

			// increment to next bit
			currentBit++;
		}

	}

	void LayerManager::RemoveLayer(std::string name)
	{
		// check if it doesn't exist
		if (collisionLayers.find(name) == collisionLayers.end())
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
		collisionLayers.erase(name);

		// i realise since i changed to map from vector, i dont have to loop like this
		// but it works
		int i = 0;
		for (auto it = indexToLayer.begin(); it != indexToLayer.end(); ++it)
		{
			if (it->second == name)
			{
				// if this is the bit that is deleted then push to removedBits
				// so we'll use this for the next layer created
				removedBits.push_back(i);

				// erase from index to layer
				indexToLayer.erase(i);
				break;
			}

			i++;
		}

	}
	uint32_t LayerManager::GetLayer(std::string name)
	{
		// check if it doesn't exist
		if (collisionLayers.find(name) == collisionLayers.end())
		{
			SLICE_LOG_ERROR(name + " doesn't exist bodoh");
			return 0;
		}
		
		return collisionLayers[name];
	}

	// might not even be using this mayb
	uint32_t LayerManager::GetLayer(int index)
	{
		if (index >= currentBit)
		{
			SLICE_LOG_ERROR(index + " doesn't exist bodoh");
			return 0;
		}

		// doesn't exist
		if (indexToLayer.find(index) == indexToLayer.end())
			return 0;

		// looks kinda cancer idk
		return collisionLayers[indexToLayer[index]];
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
			auto& collisionMask = entityGO.GetComponent<Transform>().collisionMask;

			collisionMask |= collisionLayers[name];
		}
	}

	void LayerManager::UnassignLayer(std::string name, Entity entity)
	{
		auto entityGO = FactoryInstance.GetGOByEntity(entity);

		// idk where we want to store it's collision mask but for now its in Transform
		if (entityGO.HasComponent<Transform>())
		{
			auto& collisionMask = entityGO.GetComponent<Transform>().collisionMask;

			// If it has this collision layer
			if (collisionMask & collisionLayers[name])
			{
				// remove it
				collisionMask &= ~collisionLayers[name];
			}
		}

	}
}


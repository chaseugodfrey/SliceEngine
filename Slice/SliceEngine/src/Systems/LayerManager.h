#ifndef LAYER_MANAGER_H
#define LAYER_MANAGER_H

namespace SliceEngine
{
	// only support 32 layers since we're using 32 bit int
#define MAX_LAYERS 32
#define INVALID_LAYER 33

	class LayerManager
	{
	public:
		std::unordered_map<std::string, uint32_t> collisionMask;
		std::unordered_map<std::string, uint32_t> nameToLayer;

		// default to 0
		uint32_t currentBit = 0;

		// idk if i need to create a way to access collision layers by index
		// but if i do then ill prob use this
		//std::vector<std::string> collisionKeys; 
		std::unordered_map<uint32_t, std::string> indexToLayerName;

		std::vector<uint32_t> removedBits;
		
		/// <summary>
		/// Initialize the defualt layers we wanna support
		/// Can also deserialize saved layers here probably if we do that
		/// </summary>
		void Init();

		/// <summary>
		/// For adding a new layer and assigning it a name
		/// </summary>
		/// <param name="name">Name of the layer</param>
		void AddLayer(std::string name);

		/// <summary>
		/// idk maybe need to remove layers
		/// </summary>
		/// <param name="name">name of layer to remove</param>
		void RemoveLayer(std::string name);

		uint32_t GetMask(std::string name);

		uint32_t GetMask(uint32_t index);

		uint32_t GetLayer(std::string name);

		uint32_t GetLayer(uint32_t index);

		/// <summary>
		/// Check two entity layers to see if they can interact
		/// </summary>
		/// <param name="first">First entity</param>
		/// <param name="second">Second entity</param>
		/// <returns></returns>
		bool CheckLayerInteraction(Entity first, Entity second);

		/// <summary>
		/// Assign a layer to an entity
		/// </summary>
		/// <param name="name">Name of layer</param>
		/// <param name="entity">Entity to modify</param>
		void AssignLayer(std::string name, Entity entity);

		/// <summary>
		/// Unassign a layer from an entity
		/// </summary>
		/// <param name="name">Name of the layer</param>
		/// <param name="entity">entity to modify</param>
		void UnassignLayer(std::string name, Entity entity);

		// mayb make an int version based on the index in the map or smth
	};
}


#endif
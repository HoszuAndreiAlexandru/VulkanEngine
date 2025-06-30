#include "VulkanUtils.h"
#include "../Core/Globals.h"
#include "../Core/Game/GameModel.h"

namespace Engine
{
	struct MeshHeader {
		uint32_t vertexCount;
		uint32_t indexCount;
		// Add material, AABB, LODs, etc. if needed
	};

	class VulkanModel
	{
	private:
		void loadModel(std::string modelPath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool debug = true)
		{
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string warn, err;

			auto start = std::chrono::high_resolution_clock::now();

			if (!tinyobj::LoadObj(&attrib, &shapes, nullptr, &warn, &err, modelPath.c_str()))
			{
				throw std::runtime_error(warn + err);
			}

			auto end1 = std::chrono::high_resolution_clock::now();
			auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();

			std::unordered_map<Vertex, uint32_t> uniqueVertices{};

			for (const auto& shape : shapes)
			{
				for (const auto& index : shape.mesh.indices)
				{
					Vertex vertex{};

					vertex.pos =
					{
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					// Texture coordinates
					if (index.texcoord_index >= 0 && attrib.texcoords.size() > 0)
					{
						vertex.texCoord =
						{
							attrib.texcoords[2 * index.texcoord_index + 0],
							1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
						};
					}
					else
					{
						vertex.texCoord = { 0.0f, 0.0f }; // Default UV
					}

					// Normal coordinates
					if (index.normal_index >= 0) // Check if normals are available
					{
						vertex.normal =
						{
							attrib.normals[3 * index.normal_index + 0],
							attrib.normals[3 * index.normal_index + 1],
							attrib.normals[3 * index.normal_index + 2]
						};
					}
					else
					{
						vertex.normal = { 0.0f, 0.0f, 0.0f }; // Default normal if not available
					}

					vertex.color = { 1.0f, 1.0f, 1.0f };

					if (uniqueVertices.count(vertex) == 0)
					{
						uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}

					indices.push_back(uniqueVertices[vertex]);
				}
			}

			// Step 1: Compute AABB
			glm::vec3 minPos = glm::vec3(FLT_MAX);
			glm::vec3 maxPos = glm::vec3(-FLT_MAX);

			for (const auto& v : vertices) {
				minPos = glm::min(minPos, v.pos);
				maxPos = glm::max(maxPos, v.pos);
			}

			// Step 2: Center and scale
			glm::vec3 center = (minPos + maxPos) * 0.5f;
			glm::vec3 extents = maxPos - minPos;
			float maxExtent = std::max(extents.x, std::max(extents.y, extents.z));
			float scale = 1.0f / maxExtent * 0.5f;

			for (auto& v : vertices) {
				v.pos = (v.pos - center) * scale;
			}

			auto end2 = std::chrono::high_resolution_clock::now();
			auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();

			debug && printf("1: %lld\n2: %lld", duration1, duration2);
		};

		void processShapeParallel(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape, std::vector<Vertex>& globalVertices, std::vector<uint32_t>& globalIndices)
		{
			const size_t threadCount = std::thread::hardware_concurrency();
			const auto& meshIndices = shape.mesh.indices;
			size_t totalIndices = meshIndices.size();
			size_t chunkSize = (totalIndices + threadCount - 1) / threadCount;

			std::vector<std::unordered_map<Vertex, uint32_t>> threadVertexMaps(threadCount);
			std::vector<std::vector<Vertex>> threadVertices(threadCount);
			std::vector<std::vector<uint32_t>> threadIndices(threadCount);

			auto worker = [&](int threadId) {
				size_t start = threadId * chunkSize;
				size_t end = std::min(start + chunkSize, totalIndices);

				auto& localMap = threadVertexMaps[threadId];
				auto& localVertices = threadVertices[threadId];
				auto& localIndices = threadIndices[threadId];

				for (size_t i = start; i < end; ++i)
				{
					const auto& index = meshIndices[i];
					Vertex vertex{};

					vertex.pos = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};

					if (index.normal_index >= 0) {
						vertex.normal = {
							attrib.normals[3 * index.normal_index + 0],
							attrib.normals[3 * index.normal_index + 1],
							attrib.normals[3 * index.normal_index + 2]
						};
					}
					else {
						vertex.normal = { 0.0f, 0.0f, 0.0f };
					}

					vertex.color = { 1.0f, 1.0f, 1.0f };

					uint32_t localIndex;
					if (localMap.count(vertex) == 0)
					{
						localIndex = static_cast<uint32_t>(localVertices.size());
						localMap[vertex] = localIndex;
						localVertices.push_back(vertex);
					}
					else
					{
						localIndex = localMap[vertex];
					}

					localIndices.push_back(localIndex); // Local for now
				}
				};

			std::vector<std::thread> threads;
			for (size_t i = 0; i < threadCount; ++i)
				threads.emplace_back(worker, static_cast<int>(i));

			for (auto& t : threads)
				t.join();

			// Merge step
			std::unordered_map<Vertex, uint32_t> globalMap;
			for (size_t t = 0; t < threadCount; ++t)
			{
				const auto& localMap = threadVertexMaps[t];
				const auto& localVertices = threadVertices[t];
				const auto& localIndices = threadIndices[t];

				std::unordered_map<uint32_t, uint32_t> remapTable;

				// Merge local vertices into global
				for (const auto& [vertex, localIndex] : localMap)
				{
					if (globalMap.count(vertex) == 0)
					{
						uint32_t globalIndex = static_cast<uint32_t>(globalVertices.size());
						globalMap[vertex] = globalIndex;
						globalVertices.push_back(vertex);
					}
					remapTable[localIndex] = globalMap[vertex];
				}

				// Remap local indices to global ones
				for (uint32_t localIndex : localIndices)
				{
					globalIndices.push_back(remapTable[localIndex]);
				}
			}
		}

		void saveMeshBinary(const std::string& path, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		{
			std::ofstream out(path, std::ios::binary);
			if (!out) throw std::runtime_error("Failed to open file for writing");

			MeshHeader header{};
			header.vertexCount = static_cast<uint32_t>(vertices.size());
			header.indexCount = static_cast<uint32_t>(indices.size());

			out.write(reinterpret_cast<const char*>(&header), sizeof(header));
			out.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(Vertex));
			out.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t));
		}

		void loadMeshBinary(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
		{
			std::ifstream in(path, std::ios::binary);
			if (!in) throw std::runtime_error("Failed to open file for reading");

			MeshHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(header));

			vertices.resize(header.vertexCount);
			indices.resize(header.indexCount);

			in.read(reinterpret_cast<char*>(vertices.data()), header.vertexCount * sizeof(Vertex));
			in.read(reinterpret_cast<char*>(indices.data()), header.indexCount * sizeof(uint32_t));
		}

		void loadModelWithCache(const std::string& objPath, const std::string& objName, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool debug = false)
		{
			std::string cachedPath = "Resources/Cache/Models/" + objName + ".bin";

			if (std::filesystem::exists(cachedPath)) {
				loadMeshBinary(cachedPath, vertices, indices);
				return;
			}

			// Fallback to OBJ load
			loadModel(objPath, vertices, indices, debug);
			saveMeshBinary(cachedPath, vertices, indices);
		}

		void loadModelMultiThreadedV2(std::string modelPath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool debug = false)
		{
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string warn, err;

			auto start = std::chrono::high_resolution_clock::now();

			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
			{
				throw std::runtime_error(warn + err);
			}

			auto end1 = std::chrono::high_resolution_clock::now();
			auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();

			std::unordered_map<Vertex, uint32_t> uniqueVertices{};

			for (const auto& shape : shapes)
			{
				processShapeParallel(attrib, shape, vertices, indices);
			}

			auto end2 = std::chrono::high_resolution_clock::now();
			auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();

			debug && printf("1: %lld\n2: %lld", duration1, duration2);
		}

		void createVertexBuffer(std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
		{
			VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

			copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		};

		void createIndexBuffer(std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory)
		{
			VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, indices.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

			copyBuffer(stagingBuffer, indexBuffer, bufferSize);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		};

	public:
		VulkanModel() {};

		void init() {};

		void createVulkanModel(std::string modelPath, std::string modelName)
		{
			// Calculate read time
			auto start = std::chrono::high_resolution_clock::now();

			Engine::GameModel gameModel;

			//loadModel(modelPath, gameModel.vertices, gameModel.indices);
			//loadModelMultiThreaded(modelPath, gameModel.vertices, gameModel.indices);
			//loadModelMultiThreadedV2(modelPath, gameModel.vertices, gameModel.indices);
			loadModelWithCache(modelPath, modelName, gameModel.vertices, gameModel.indices);
			createVertexBuffer(gameModel.vertices, gameModel.vertexBuffer, gameModel.vertexBufferMemory);
			createIndexBuffer(gameModel.indices, gameModel.indexBuffer, gameModel.indexBufferMemory);

			gameModel.setName(modelName);

			// End read time
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			std::cout << "\nModel: " << modelName << "\n    Disk load time: " << duration << " ms\n";
			//printf("Model: %d, load time: %d ms.\n", modelName, duration);

			gameModel.createCustomBVH(gameManager.models.size());

			//GameModel gameModel(vertices, indices, vertexBuffer, vertexBufferMemory, indexBuffer, indexBufferMemory);
			gameManager.models[modelName] = gameModel;
		};

		void createVulkanModels(std::string folderPath)
		{
			try
			{
				for (const auto& entry : std::filesystem::directory_iterator(folderPath))
				{
					const std::filesystem::path filePath = entry.path();
					const std::string fileName = filePath.stem().string(); // File name without extension
					const std::string fileExtension = filePath.extension().string();

					if (fileExtension == ".obj")
					{
						this->createVulkanModel(filePath.string(), fileName);
					}
				}
			}
			catch (const std::filesystem::filesystem_error& err)
			{
				std::cerr << "Filesystem error: " << err.what() << std::endl;
			}
		};
	};
};
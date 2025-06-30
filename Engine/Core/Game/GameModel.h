#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../../Vulkan/VulkanTypes.h"
#include "../../Vulkan/VulkanGlobals.h"

namespace Engine
{
	struct AABB {
		glm::vec3 min;
		glm::vec3 max;

		AABB() {
			min = glm::vec3(std::numeric_limits<float>::max());
			max = glm::vec3(std::numeric_limits<float>::lowest());
		}

		void expand(const glm::vec3& point) {
			min = glm::min(min, point);
			max = glm::max(max, point);
		}

		void expand(const AABB& box) {
			expand(box.min);
			expand(box.max);
		}

		int longestAxis() const {
			glm::vec3 extents = max - min;
			if (extents.x > extents.y && extents.x > extents.z) return 0;
			if (extents.y > extents.z) return 1;
			return 2;
		}
	};

	class GameModel
	{
	private:
		int maxLeafSize = 2;
		std::vector<int> triIndices;
		std::string name = "model";

		void createTriangles()
		{
			triangles.clear();
			for (size_t i = 0; i < indices.size(); i += 3)
			{
				Triangle triangle;
				triangle.v0 = glm::vec4(vertices[indices[i]].pos, 1.0f);
				triangle.v1 = glm::vec4(vertices[indices[i + 1]].pos, 1.0f);
				triangle.v2 = glm::vec4(vertices[indices[i + 2]].pos, 1.0f);
				triangle.centroid = (triangle.v0 + triangle.v1 + triangle.v2) / 3.0f;
				triangles.push_back(triangle);
			}
		}

		int buildNaive(int start, int end)
		{
			BVHNode node;
			node.boundMin = glm::vec3(FLT_MAX);
			node.boundMax = glm::vec3(-FLT_MAX);
			for (int i = start; i < end; ++i)
			{
				node.expand(triangles[i].v0), node.expand(triangles[i].v1), node.expand(triangles[i].v2);
			}

			int count = end - start;
			if (count <= maxLeafSize)
			{
				node.firstTriangle = start;
				node.triangleCount = count;
				int index = (int)nodes.size();
				nodes.push_back(node);
				return index;
			}

			// Partition along longest axis
			glm::vec3 extent = node.boundMax - node.boundMin;
			int axis = extent.x > extent.y ? (extent.x > extent.z ? 0 : 2) : (extent.y > extent.z ? 1 : 2);
			std::sort(triangles.begin() + start, triangles.begin() + end, [&](const Triangle& a, const Triangle& b)
				{
					float ca = (a.v0[axis] + a.v1[axis] + a.v2[axis]) / 3.0f;
					float cb = (b.v0[axis] + b.v1[axis] + b.v2[axis]) / 3.0f;
					return ca < cb;
				});

			int mid = start + count / 2;
			int left = buildNaive(start, mid);
			int right = buildNaive(mid, end);

			node.left = left;
			node.right = right;
			int index = (int)nodes.size();
			nodes.push_back(node);
			return index;
		}

		int buildBVH()
		{
			int triangleCount = static_cast<int>(triangles.size());
			std::atomic<int> atomicNodesUsed(1);

			// Step 1: Create triangle index list and compute centroids
			std::vector<uint32_t> triIdx(triangleCount);
			for (int i = 0; i < triangleCount; ++i)
			{
				triIdx[i] = i;
				Triangle& tri = triangles[i];
				tri.centroid = glm::vec4((glm::vec3(tri.v0) + glm::vec3(tri.v1) + glm::vec3(tri.v2)) / 3.0f, 0);
			}

			// Step 2: Temporary node storage, using naive logic
			std::vector<BVHNode> tempNodes(triangleCount * 2);
			int nodesUsed = 1;
			int rootIdx = 0;

			BVHNode& root = tempNodes[rootIdx];
			root.firstTriangle = 0;
			root.triangleCount = triangleCount;
			updateNodeBounds(tempNodes, triIdx, rootIdx);
			subdivideBVH(tempNodes, triIdx, 0, maxLeafSize, atomicNodesUsed, 0);

			// Step 3: Remap triangles into triangle list in correct order
			std::vector<Triangle> orderedTriangles;
			orderedTriangles.reserve(triangleCount);
			std::vector<int> indexMap(triangleCount); // oldIndex -> newIndex

			for (int i = 0; i < triangleCount; ++i)
			{
				indexMap[triIdx[i]] = static_cast<int>(orderedTriangles.size());
				orderedTriangles.push_back(triangles[triIdx[i]]);
			}

			nodesUsed = atomicNodesUsed.load();

			// Step 4: Rewrite firstTriangle to match new triangle order
			for (int i = 0; i < nodesUsed; ++i)
			{
				BVHNode& node = tempNodes[i];
				if (node.triangleCount > 0)
				{
					int originalStart = node.firstTriangle;
					int newStart = indexMap[triIdx[originalStart]];
					node.firstTriangle = newStart;
				}
			}

			// Step 5: Save final results
			nodes.clear();
			nodes = std::vector<BVHNode>(tempNodes.begin(), tempNodes.begin() + nodesUsed);
			triangles = std::move(orderedTriangles);

			return rootIdx;
		}

		void updateNodeBounds(std::vector<BVHNode>& nodes, const std::vector<uint32_t>& triIdx, uint32_t nodeIdx)
		{
			BVHNode& node = nodes[nodeIdx];
			glm::vec3 minBound(FLT_MAX), maxBound(-FLT_MAX);

			for (int i = 0; i < node.triangleCount; ++i)
			{
				const Triangle& tri = triangles[triIdx[node.firstTriangle + i]];
				minBound = glm::min(minBound, glm::min(glm::vec3(tri.v0), glm::min(glm::vec3(tri.v1), glm::vec3(tri.v2))));
				maxBound = glm::max(maxBound, glm::max(glm::vec3(tri.v0), glm::max(glm::vec3(tri.v1), glm::vec3(tri.v2))));
			}

			node.boundMin = minBound;
			node.boundMax = maxBound;
		}

		void subdivideBVH(std::vector<BVHNode>& nodes, std::vector<uint32_t>& triIdx, uint32_t nodeIdx, int maxLeafSize, std::atomic<int>& atomicNodesUsed, int depth = 0)
		{
			BVHNode& node = nodes[nodeIdx];
			if (node.triangleCount <= maxLeafSize) return;

			glm::vec3 extent = node.boundMax - node.boundMin;
			int axis = (extent.y > extent.x) ? (extent.y > extent.z ? 1 : 2) : (extent.x > extent.z ? 0 : 2);
			float splitPos = node.boundMin[axis] + extent[axis] * 0.5f;

			int i = node.firstTriangle;
			int j = i + node.triangleCount - 1;

			while (i <= j)
			{
				if (triangles[triIdx[i]].centroid[axis] < splitPos)
					++i;
				else
					std::swap(triIdx[i], triIdx[j--]);
			}

			int leftCount = i - node.firstTriangle;
			if (leftCount == 0 || leftCount == node.triangleCount) return;

			int leftIdx = atomicNodesUsed++;
			int rightIdx = atomicNodesUsed++;

			nodes[leftIdx].firstTriangle = node.firstTriangle;
			nodes[leftIdx].triangleCount = leftCount;
			updateNodeBounds(nodes, triIdx, leftIdx);

			nodes[rightIdx].firstTriangle = i;
			nodes[rightIdx].triangleCount = node.triangleCount - leftCount;
			updateNodeBounds(nodes, triIdx, rightIdx);

			node.left = leftIdx;
			node.right = rightIdx;
			node.firstTriangle = -1;
			node.triangleCount = 0;

			if (depth < 3)  // Parallel spawn limit
			{
				auto leftFuture = std::async(std::launch::async, [=, &nodes, &triIdx, &atomicNodesUsed]() {
					subdivideBVH(nodes, triIdx, leftIdx, maxLeafSize, atomicNodesUsed, depth + 1);
					});
				auto rightFuture = std::async(std::launch::async, [=, &nodes, &triIdx, &atomicNodesUsed]() {
					subdivideBVH(nodes, triIdx, rightIdx, maxLeafSize, atomicNodesUsed, depth + 1);
					});
				leftFuture.get();
				rightFuture.get();
			}
			else
			{
				subdivideBVH(nodes, triIdx, leftIdx, maxLeafSize, atomicNodesUsed, depth + 1);
				subdivideBVH(nodes, triIdx, rightIdx, maxLeafSize, atomicNodesUsed, depth + 1);
			}
		}

		#pragma region BVH2
		int buildBVH2()
		{
			nodes.clear();
			if (triangles.empty()) return -1;
			return buildBVHRecursive(0, (int)triangles.size());
		}

		int buildBVHRecursive(int start, int end)
		{
			int triangleCount = end - start;
			AABB bounds, centroidBounds;

			for (int i = start; i < end; ++i)
			{
				const glm::vec3 centroid = glm::vec3(triangles[i].centroid);
				centroidBounds.expand(centroid);

				const glm::vec3 v0 = glm::vec3(triangles[i].v0);
				const glm::vec3 v1 = glm::vec3(triangles[i].v1);
				const glm::vec3 v2 = glm::vec3(triangles[i].v2);
				bounds.expand(v0);
				bounds.expand(v1);
				bounds.expand(v2);
			}

			if (nodes.size() >= 144046)
			{
				//std::cout << "" << std::endl;
			}

			BVHNode node;
			node.boundMin = bounds.min;
			node.boundMax = bounds.max;

			if (triangleCount <= maxLeafSize)
			{
				node.firstTriangle = (int)nodes.size(); // index into the flattened triangle array
				node.triangleCount = triangleCount;
				nodes.push_back(node);

				// Leaf triangles should be stored in order
				for (int i = start; i < end; ++i)
				{
					//triangles[node.firstTriangle + i - start] = triangles[i];
				}

				return (int)nodes.size() - 1;
			}

			// Split
			int axis = centroidBounds.longestAxis();
			float splitPos = 0.5f * (centroidBounds.min[axis] + centroidBounds.max[axis]);

			auto mid = std::partition(triangles.begin() + start, triangles.begin() + end,
				[axis, splitPos](const Triangle& t) {
					return t.centroid[axis] < splitPos;
				});

			int midIndex = (int)(mid - triangles.begin());

			// If partition failed (all on one side), do a median split
			if (midIndex == start || midIndex == end)
			{
				midIndex = start + (triangleCount / 2);
				std::nth_element(triangles.begin() + start, triangles.begin() + midIndex, triangles.begin() + end,
					[axis](const Triangle& a, const Triangle& b) {
						return a.centroid[axis] < b.centroid[axis];
					});
			}

			int currentIndex = (int)nodes.size();
			nodes.push_back(BVHNode()); // placeholder

			int leftChild = buildBVHRecursive(start, midIndex);
			int rightChild = buildBVHRecursive(midIndex, end);

			// Fill current node
			nodes[currentIndex] = node;
			nodes[currentIndex].left = leftChild;
			nodes[currentIndex].right = rightChild;

			return currentIndex;
		}
        #pragma endregion BVH2

		//
		bool validateBVH(int nodeIndex)
		{
			if (nodeIndex < 0 || nodeIndex >= nodes.size())
			{
				return false;
			}

			const BVHNode& node = nodes[nodeIndex];

			// Check AABB validity
			for (int i = 0; i < 3; ++i)
			{
				if (node.boundMin[i] > node.boundMax[i])
				{
					printf("Invalid AABB at node %d\n", nodeIndex);
					return false;
				}
			}

			// Leaf node check
			bool isLeaf = (node.triangleCount > 0);
			if (isLeaf)
			{
				if (node.left != -1 || node.right != -1)
				{
					printf("Leaf node %d has children\n", nodeIndex);
					return false;
				}
			}
			else
			{
				// Internal node should have valid children
				if (node.left < 0 || node.right < 0)
				{
					printf("Internal node %d missing children\n", nodeIndex);
					return false;
				}
				if (!validateBVH(node.left) || !validateBVH(node.right))
				{
					return false;
				}
			}

			return true;
		}

		bool validateBVHTriangles()
		{
			for (const BVHNode& node : nodes)
			{
				for(int i = node.firstTriangle; i < node.firstTriangle + node.triangleCount; ++i)
				{
					if (i < 0 || i >= triangles.size())
					{
						printf("Invalid triangle index %d in node %lld.\n", i, &node - &nodes[0]);
						return false;
					}

					const Triangle& tri = triangles[i];

					if (!triangleInAABB(tri, node))
					{
						printf("Invalid triangle bounds for node %d: index %lld.\n", i, &node - &nodes[0]);
						return false;
					}
				}
			}

			return true;
		}

		bool triangleInAABB(const Triangle& tri, const BVHNode& node) 
		{
			glm::vec3 min = node.boundMin;
			glm::vec3 max = node.boundMax;

			auto inside = [&](const glm::vec3& v)
			{
				return all(glm::greaterThanEqual(v, min)) && all(glm::lessThanEqual(v, max));
			};

			return inside(glm::vec3(tri.v0)) && inside(glm::vec3(tri.v1)) && inside(glm::vec3(tri.v2));
		}

	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		int localRoot = 0;
		int bvhTriangleIndex = 0;
		int bvhRootNodeIndex = 0;
		std::vector<Triangle> triangles; // To be appended to the GPU buffer
		std::vector<BVHNode> nodes; // To be appended to the GPU buffer

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		GameModel() 
			: vertexBuffer(VK_NULL_HANDLE),            // Initialize to default values
			vertexBufferMemory(VK_NULL_HANDLE),		   // VK_NULL_HANDLE is used to initialize Vulkan handles
			indexBuffer(VK_NULL_HANDLE),
			indexBufferMemory(VK_NULL_HANDLE)
		{};

		GameModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
			: vertices(vertices),
			indices(indices),
			vertexBuffer(VK_NULL_HANDLE),            // Initialize to default values
			vertexBufferMemory(VK_NULL_HANDLE),		   // VK_NULL_HANDLE is used to initialize Vulkan handles
			indexBuffer(VK_NULL_HANDLE),
			indexBufferMemory(VK_NULL_HANDLE)
		{};

		GameModel
		(
			std::vector<Vertex> vertices, 
			std::vector<uint32_t> indices, 
			VkBuffer vertexBuffer, 
			VkDeviceMemory vertexBufferMemory, 
			VkBuffer indexBuffer, 
			VkDeviceMemory indexBufferMemory
		)
			: vertices(vertices),
			indices(indices),
			vertexBuffer(vertexBuffer),
			vertexBufferMemory(vertexBufferMemory),
			indexBuffer(indexBuffer),
			indexBufferMemory(indexBufferMemory)
		{};

		void createCustomBVH(int meshIndex, bool showDebugMessages = false, bool showOnlyBuildTimes = false)
		{
			// Calculate algorithm time
			auto start = std::chrono::high_resolution_clock::now();

			int bvhNodesSize = (int)bvhNodes.size();
			int trianglesSize = (int)bvhTriangles.size();

			// Create triangles from the vertices and indices
			createTriangles();
			auto endTriangles = std::chrono::high_resolution_clock::now();

			// Create BVH nodes
			//localRoot = buildNaive(0, triangles.size());
			localRoot = buildBVH();
			//localRoot = buildBVH2();

			// End algorithm time
			auto endBvh = std::chrono::high_resolution_clock::now();
			auto durationTriangles = std::chrono::duration_cast<std::chrono::milliseconds>(endTriangles - start).count();
			auto durationBvh = std::chrono::duration_cast<std::chrono::milliseconds>(endBvh - endTriangles).count();

			bool ok1 = validateBVH(localRoot);
			bool ok2 = validateBVHTriangles();
			bool ok3 = localRoot >= 0 && localRoot < nodes.size();
			showDebugMessages && printf("BVH validation: %s\n", ok1 && ok2 && ok3 ? "OK" : "FAILED");

			// Add base offset to the local bvh children nodes
			for (auto& node : nodes)
			{
				if (node.left != -1) node.left += bvhNodesSize;
				if (node.right != -1) node.right += bvhNodesSize;
			}

			// Prepare for GPU buffers
			bvhRootNodeIndex = bvhNodes.size() + localRoot;
			bvhTriangleIndex = bvhTriangles.size();

			// Add to GPU buffers
			bvhNodes.insert(bvhNodes.end(), nodes.begin(), nodes.end());
			bvhTriangles.insert(bvhTriangles.end(), triangles.begin(), triangles.end());

			if (showOnlyBuildTimes)
			{
				printf("    BVH nodes build time: %lld ms\n", durationBvh);
				printf("    Triangles build time: %lld ms\n", durationTriangles);
			}
			else if (showDebugMessages)
			{
				printf("BVH nodes\n    Count: %zd\n    Build time: %lld ms\n    Global root index: %d\n", nodes.size(), durationBvh, bvhRootNodeIndex);
				printf("Triangles\n    Count: %zd\n    Build time: %lld ms\n    Global start index: %d\n    Global end index: %lld\n", triangles.size(), durationTriangles, trianglesSize, trianglesSize + triangles.size());

				printf("Buffer sizes\n");
				printf("    BVH: %d -> %lld\n", bvhNodesSize, bvhNodes.size());
				printf("    Triangles: %d -> %lld\n", trianglesSize, bvhTriangles.size());
			}
		}

		void setName(const std::string& modelName)
		{
			name = modelName;
		};

		std::string getName() const
		{
			return name;
		};
	};
};
#pragma once
#define FNL_IMPL
#include "FastNoiseLite.hpp"
#include <condition_variable>
#include <cstdint>
#include <glm/glm.hpp>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//=================
//  Materials
//-----------------
#define MAT_AIR 0
#define MAT_STONE 1
#define MAT_STONE2 2
#define MAT_STONE3 3
#define MAT_GRASS 9
#define MAT_FLOWER 10
#define MAT_WATER 19
#define MAT_PLANT_CORE 20
// ================
#define voxel_mat(voxel) ((voxel) & 0b11111)
#define voxel_base(voxel) (((voxel) & 0b11100000) >> 5)
#define MAT_IS_STONE(mat) ((mat) >= MAT_STONE && (mat) <= MAT_STONE3)
#define MAT_HAS_COLLISION(mat) ((mat) > MAT_AIR && (mat) < MAT_FLOWER)

class VoxelWorld {
public:
  struct VoxelChunk {
    uint8_t *data;
    glm::ivec3 position;
    bool inQueue;
  };

  VoxelWorld(std::unique_ptr<class VulkanContext> &vulkanContext,
             std::unique_ptr<class CommandManager> &commandManager);
  ~VoxelWorld();

  void generateTerrain();
  void startWorkers(int numThreads);
  void stopWorkers();
  void requestChunk(uint16_t chunkID, float priority);
  void sortChunks();
  void updateVoxelChunkMap(int modValue, int offset);
  void updateVoxels(VkCommandBuffer commandBuffer,
                    std::unique_ptr<VulkanContext> &vulkanContext,
                    int currentImage);
  void chunkWorker();
  void generateChunk(VoxelChunk &chunk);
  uint8_t *getChunk(glm::vec3 position);
  uint16_t getChunkID(glm::vec3 position);

  // private: TODO: Put this back in
  glm::ivec3 chunkPosition;
  bool chunkSort(uint16_t a, uint16_t b);

  std::vector<std::thread> workers;
  std::vector<uint16_t> chunkQueue;
  std::unordered_set<uint16_t> activeChunks;
  std::mutex queueMutex;
  std::condition_variable queueCond;
  bool stopThreads;

  std::vector<VkImage> voxelTexture;
  std::vector<VkDeviceMemory> voxelTexturesMemory;
  std::vector<VkImageView> voxelImageView;

  std::vector<uint16_t> chunkUpdateQueue;

  std::vector<VoxelChunk> voxelData;
  VkSampler voxelTextureSampler;
  std::vector<VkBuffer> voxelStagingBuffer;
  std::vector<VkDeviceMemory> voxelStagingBufferMemory;
  std::vector<VkImage> voxelChunkMapTexture;
  std::vector<VkDeviceMemory> voxelChunkMapTexturesMemory;
  std::vector<VkImageView> voxelChunkMapImageView;
  std::vector<VkBuffer> voxelChunkMapStagingBuffer;
  std::vector<VkDeviceMemory> voxelChunkMapStagingBufferMemory;
  uint16_t *voxelChunkMapData;

  FastNoiseLite noise;

  friend class Raytracer;
};
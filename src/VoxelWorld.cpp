#include "VoxelWorld.hpp"
#include "VkZero/image.hpp"
#include "VkZero/Internal/resource_manager_internal.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

VoxelWorld::VoxelWorld()
    : voxelChunkMapImage{8,
                         8,
                         8,
                         VkZero::Format::R16Uint,
                         VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                             VK_IMAGE_USAGE_SAMPLED_BIT |
                             VK_IMAGE_USAGE_STORAGE_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT} {
  chunkPosition = glm::ivec3(0, 0, 0);
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  for (int i = 0; i < 512; i++) {
    voxelImages.emplace_back(128, 128, 128, VkZero::Format::R8Uint,
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                 VK_IMAGE_USAGE_STORAGE_BIT |
                                 VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  }

  startWorkers(30);
  for (int i = 0; i < 512; i++) {
    requestChunk(i, 1.0f);
  }
  sortChunks();
  std::this_thread::sleep_for(std::chrono::seconds(10));

  // Voxel Data

  voxelChunkMapData = new uint16_t[8 * 8 * 8]();

  for (int i = 0; i < 8 * 8 * 8; i++) {
    voxelChunkMapData[i] = i;
  }
}
VoxelWorld::~VoxelWorld() {}

bool VoxelWorld::chunkSort(uint16_t a, uint16_t b) {
  glm::vec3 cameraPosition{-512, -512, -512};
  glm::vec3 cameraGlobal = (glm::vec3(chunkPosition) * 128.0f) - cameraPosition;
  return glm::distance(cameraGlobal, glm::vec3(voxelData[a].position)) >
         glm::distance(cameraGlobal, glm::vec3(voxelData[b].position));
}

void VoxelWorld::generateTerrain() {
  auto it = std::chrono::system_clock::now();
#define vox(x, y, z)                                                           \
  voxelData[(int)std::floor((float)(z) / 128.0f) * 64 +                        \
            (int)std::floor((float)(y) / 128.0f) * 8 +                         \
            (int)std::floor((float)(x) / 128.0f)]                              \
      .data[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]
#define voxChunk(chunk, x, y, z)                                               \
  chunk[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]

#define TERRAIN_SCALE 2
#define TERRAIN_AREA (1024)

  const int WORKERS = 20;
  const int SLICE_THICKNESS = TERRAIN_AREA / WORKERS;

  std::thread *threads[WORKERS]{};

  for (int i = 0; i < WORKERS; i++) {
    threads[i]->join();
  }

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - it);
}
void VoxelWorld::startWorkers(int numThreads) {
  voxelData.resize(512);
  chunkQueue.resize(513);
  chunkQueue[0] = 0;
  chunkUpdateQueue.resize(513);
  chunkUpdateQueue[0] = 0;

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      for (int z = 0; z < 8; z++) {
        size_t chunkIndex = z * 64 + y * 8 + x;
        voxelData[chunkIndex] = {};
        voxelData[chunkIndex].data = new uint8_t[128 * 128 * 128]();
        voxelData[chunkIndex].position = glm::ivec3(x * 128, y * 128, z * 128);
      }
    }
  }

  for (int i = 0; i < numThreads; i++) {
    workers.emplace_back([this] { chunkWorker(); });
  }
}
void VoxelWorld::stopWorkers() {
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    stopThreads = true;
  }
  queueCond.notify_all();
  for (auto &worker : workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}
void VoxelWorld::requestChunk(uint16_t chunkID, float priority) {
  // std::fill(voxelData[chunkID].data, voxelData[chunkID].data + 128 * 128 *
  // 128, 0);
  std::lock_guard<std::mutex> lock(queueMutex);
  // chunkGenerationID[chunkID]++; // Increment generation ID
  if (voxelData[chunkID].inQueue)
    return;
  chunkQueue[++chunkQueue[0]] = chunkID;
  voxelData[chunkID].inQueue = true;
  queueCond.notify_one();
}
void VoxelWorld::sortChunks() {
  std::lock_guard<std::mutex> lock(queueMutex);
  if (chunkQueue[0] > 5)
    std::sort(chunkQueue.begin() + 1, chunkQueue.begin() + chunkQueue[0],
              [this](uint16_t a, uint16_t b) { return chunkSort(a, b); });
}
void VoxelWorld::updateVoxelChunkMap(int modValue, int offset) {
  switch (offset) {
  case 1:
    chunkPosition += glm::ivec3(1, 0, 0);
    break;
  case 7:
    chunkPosition -= glm::ivec3(1, 0, 0);
    break;
  case 8:
    chunkPosition += glm::ivec3(0, 1, 0);
    break;
  case 56:
    chunkPosition += glm::ivec3(0, -1, 0);
    break;
  case 64:
    chunkPosition += glm::ivec3(0, 0, 1);
    break;
  case 448:
    chunkPosition += glm::ivec3(0, 0, -1);
    break;

  default:
    break;
  }

  const int chunkSize = 8 * 128; // 1024
  for (int z = 0; z < 8; z++) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        int index = z * 8 * 8 + y * 8 + x;
        int chunk = voxelChunkMapData[index];
        int chunkAxis = chunk % modValue;

        uint16_t chunkID =
            (chunk - chunkAxis) + (chunkAxis + offset) % modValue;
        switch (modValue) {
        case 8:
          if (offset == 1 && x == 7) {
            voxelData[chunkID].position.x += chunkSize;
            requestChunk(chunkID, 0.0f);
          } else if (offset == 7 && x == 0) {
            voxelData[chunkID].position.x -= chunkSize;
            requestChunk(chunkID, 0.0f);
          }
          break;
        case 64:
          if (offset == 8 && y == 7) {
            voxelData[chunkID].position.y += chunkSize;
            requestChunk(chunkID, 0.0f);
          } else if (offset == 56 && y == 0) {
            voxelData[chunkID].position.y -= chunkSize;
            requestChunk(chunkID, 0.0f);
          }
          break;
        case 512:
          if (offset == 64 && z == 7) {
            voxelData[chunkID].position.z += chunkSize;
            requestChunk(chunkID, 0.0f);
          } else if (offset == 448 && z == 0) {
            voxelData[chunkID].position.z -= chunkSize;
            requestChunk(chunkID, 0.0f);
          }
          break;
        }

        voxelChunkMapData[index] = chunkID;
      }
    }
  }
}
void VoxelWorld::updateVoxels(VkCommandBuffer commandBuffer, int currentImage) {
  voxelChunkMapImage.changeLayout(commandBuffer,
                                  VkZero::ImageLayout::TransferDstOptimal, 0);

  for (int i = 0; i < 16; i++) {
    if (chunkUpdateQueue[0] == 0) {
      break;
    }

    uint16_t ID = chunkUpdateQueue[chunkUpdateQueue[0]--];

    voxelImages[ID].changeLayout(commandBuffer,
                                 VkZero::ImageLayout::TransferDstOptimal, 0);
    voxelImages[ID].write(commandBuffer, (char *)voxelData[ID].data, 0);
    voxelImages[ID].changeLayout(commandBuffer, VkZero::ImageLayout::General,
                                 0);
  }

  // TODO I need to change the format sizing in the write for this since its 16
  // instead of 8
  //  voxelChunkMapImage is a StagedSharedImage with only 1 image, so index must
  //  be 0
  voxelChunkMapImage.write(commandBuffer, (char *)voxelChunkMapData, 0);
  voxelChunkMapImage.changeLayout(commandBuffer, VkZero::ImageLayout::General,
                                  0);
}
void VoxelWorld::chunkWorker() {
  while (!stopThreads) {
    uint16_t chunkID;
    uint32_t genID;

    {
      std::unique_lock<std::mutex> lock(queueMutex);
      queueCond.wait(lock, [&] { return chunkQueue[0] != 0 || stopThreads; });
      if (stopThreads)
        return;
      if (chunkQueue[0] > 0) {
        chunkID = chunkQueue[chunkQueue[0]--];
      } else {
        continue; // Recheck condition
      }
    }
    voxelData[chunkID].inQueue = false;
    generateChunk(voxelData[chunkID]);
    chunkUpdateQueue[++chunkUpdateQueue[0]] = chunkID;
  }
}
void VoxelWorld::generateChunk(VoxelChunk &chunk) {
  glm::ivec3 position = chunk.position;
  uint8_t *chunkData = chunk.data;
#pragma omp parallel for collapse(2)
  for (int z = 0; z < 128; z++) {
    for (int y = 0; y < 128; y++) {
      for (int x = 0; x < 128; x++) {
        glm::ivec3 samplePosition = position + glm::ivec3(x, y, z);
        uint8_t base = 0; // 0b11100000;
        float v = (noise.GetNoise(samplePosition.x * 0.1f * TERRAIN_SCALE,
                                  samplePosition.y * 0.1f * TERRAIN_SCALE,
                                  samplePosition.z * 0.4f * TERRAIN_SCALE) +
                   noise.GetNoise(samplePosition.x * 1.5f * TERRAIN_SCALE,
                                  samplePosition.y * 1.5f * TERRAIN_SCALE,
                                  samplePosition.z * 1.0f * TERRAIN_SCALE) *
                       0.03) /
                  1.03f;
        float v_above =
            (noise.GetNoise(samplePosition.x * 0.1f * TERRAIN_SCALE,
                            samplePosition.y * 0.1f * TERRAIN_SCALE,
                            (samplePosition.z - 1) * 0.4f * TERRAIN_SCALE) +
             noise.GetNoise(samplePosition.x * 1.5f * TERRAIN_SCALE,
                            samplePosition.y * 1.5f * TERRAIN_SCALE,
                            (samplePosition.z - 1) * 1.0f * TERRAIN_SCALE) *
                 0.03) /
            1.03f;
        int material = v <= 0.1;
        if (material != MAT_AIR) {
          if ((v_above <= 0.1)) {
            float v = noise.GetNoise(x * 10.0f * TERRAIN_SCALE,
                                     y * 10.0f * TERRAIN_SCALE,
                                     z * 10.0f * TERRAIN_SCALE) *
                          0.7f +
                      noise.GetNoise(x * 5.0f * TERRAIN_SCALE,
                                     y * 5.0f * TERRAIN_SCALE,
                                     z * 5.0f * TERRAIN_SCALE) *
                          0.3f;
            if (v > 0.33) {
              material = MAT_STONE;
            } else if (v > -0.33) {
              material = MAT_STONE2;
            } else {
              material = MAT_STONE3;
            }
          }
        }

        voxChunk(chunkData, x, y, z) = base | material;
      }
    }
  }

// Set the base
#pragma omp parallel for collapse(2)
  for (int x = 0; x < 128; x += 2) {
    for (int y = 0; y < 128; y += 2) {
      for (int z = 0; z < 128; z += 2) {
        if (voxel_mat(voxChunk(chunkData, x, y, z)) == MAT_AIR &&
            voxel_mat(voxChunk(chunkData, x + 1, y, z)) == MAT_AIR &&
            voxel_mat(voxChunk(chunkData, x, y + 1, z)) == MAT_AIR &&
            voxel_mat(voxChunk(chunkData, x, y, z + 1)) == MAT_AIR &&
            voxel_mat(voxChunk(chunkData, x + 1, y, z + 1)) == MAT_AIR &&
            voxel_mat(voxChunk(chunkData, x + 1, y + 1, z)) == MAT_AIR &&
            voxel_mat(voxChunk(chunkData, x, y + 1, z + 1)) == MAT_AIR &&
            voxel_mat(voxChunk(chunkData, x + 1, y + 1, z + 1)) == MAT_AIR) {
          uint8_t value = 1 << 5;
          voxChunk(chunkData, x, y, z) = value;
          voxChunk(chunkData, x + 1, y, z) = value;
          voxChunk(chunkData, x, y + 1, z) = value;
          voxChunk(chunkData, x, y, z + 1) = value;
          voxChunk(chunkData, x + 1, y, z + 1) = value;
          voxChunk(chunkData, x + 1, y + 1, z) = value;
          voxChunk(chunkData, x, y + 1, z + 1) = value;
          voxChunk(chunkData, x + 1, y + 1, z + 1) = value;
        }
      }
    }
  }
#pragma omp parallel for collapse(2)
  for (int x = 0; x < 128; x += 4) {
    for (int y = 0; y < 128; y += 4) {
      for (int z = 0; z < 128; z += 4) {
        if (voxel_base(voxChunk(chunkData, x, y, z)) == 1 &&
            voxel_base(voxChunk(chunkData, x + 2, y, z)) == 1 &&
            voxel_base(voxChunk(chunkData, x, y + 2, z)) == 1 &&
            voxel_base(voxChunk(chunkData, x, y, z + 2)) == 1 &&
            voxel_base(voxChunk(chunkData, x + 2, y, z + 2)) == 1 &&
            voxel_base(voxChunk(chunkData, x + 2, y + 2, z)) == 1 &&
            voxel_base(voxChunk(chunkData, x, y + 2, z + 2)) == 1 &&
            voxel_base(voxChunk(chunkData, x + 2, y + 2, z + 2)) == 1) {
          uint8_t value = 2 << 5;
          for (int x2 = 0; x2 < 4; x2++) {
            for (int y2 = 0; y2 < 4; y2++) {
              for (int z2 = 0; z2 < 4; z2++) {
                voxChunk(chunkData, x + x2, y + y2, z + z2) = value;
              }
            }
          }
        }
      }
    }
  }

#pragma omp parallel for collapse(2)
  for (int x = 0; x < 128; x += 8) {
    for (int y = 0; y < 128; y += 8) {
      for (int z = 0; z < 128; z += 8) {
        if (voxel_base(voxChunk(chunkData, x, y, z)) == 2 &&
            voxel_base(voxChunk(chunkData, x + 4, y, z)) == 2 &&
            voxel_base(voxChunk(chunkData, x, y + 4, z)) == 2 &&
            voxel_base(voxChunk(chunkData, x, y, z + 4)) == 2 &&
            voxel_base(voxChunk(chunkData, x + 4, y, z + 4)) == 2 &&
            voxel_base(voxChunk(chunkData, x + 4, y + 4, z)) == 2 &&
            voxel_base(voxChunk(chunkData, x, y + 4, z + 4)) == 2 &&
            voxel_base(voxChunk(chunkData, x + 4, y + 4, z + 4)) == 2) {
          uint8_t value = 3 << 5;
          for (int x2 = 0; x2 < 8; x2++) {
            for (int y2 = 0; y2 < 8; y2++) {
              for (int z2 = 0; z2 < 8; z2++) {
                voxChunk(chunkData, x + x2, y + y2, z + z2) = value;
              }
            }
          }
        }
      }
    }
  }
#pragma omp parallel for collapse(2)
  for (int x = 0; x < 128; x += 16) {
    for (int y = 0; y < 128; y += 16) {
      for (int z = 0; z < 128; z += 16) {
        if (voxel_base(voxChunk(chunkData, x, y, z)) == 3 &&
            voxel_base(voxChunk(chunkData, x + 8, y, z)) == 3 &&
            voxel_base(voxChunk(chunkData, x, y + 8, z)) == 3 &&
            voxel_base(voxChunk(chunkData, x, y, z + 8)) == 3 &&
            voxel_base(voxChunk(chunkData, x + 8, y, z + 8)) == 3 &&
            voxel_base(voxChunk(chunkData, x + 8, y + 8, z)) == 3 &&
            voxel_base(voxChunk(chunkData, x, y + 8, z + 8)) == 3 &&
            voxel_base(voxChunk(chunkData, x + 8, y + 8, z + 8)) == 3) {
          uint8_t value = 4 << 5;
          for (int x2 = 0; x2 < 16; x2++) {
            for (int y2 = 0; y2 < 16; y2++) {
              for (int z2 = 0; z2 < 16; z2++) {
                voxChunk(chunkData, x + x2, y + y2, z + z2) = value;
              }
            }
          }
        }
      }
    }
  }
#pragma omp parallel for collapse(2)
  for (int x = 0; x < 128; x += 32) {
    for (int y = 0; y < 128; y += 32) {
      for (int z = 0; z < 128; z += 32) {
        if (voxel_base(voxChunk(chunkData, x, y, z)) == 4 &&
            voxel_base(voxChunk(chunkData, x + 16, y, z)) == 4 &&
            voxel_base(voxChunk(chunkData, x, y + 16, z)) == 4 &&
            voxel_base(voxChunk(chunkData, x, y, z + 16)) == 4 &&
            voxel_base(voxChunk(chunkData, x + 16, y, z + 16)) == 4 &&
            voxel_base(voxChunk(chunkData, x + 16, y + 16, z)) == 4 &&
            voxel_base(voxChunk(chunkData, x, y + 16, z + 16)) == 4 &&
            voxel_base(voxChunk(chunkData, x + 16, y + 16, z + 16)) == 4) {
          uint8_t value = 5 << 5;
          for (int x2 = 0; x2 < 32; x2++) {
            for (int y2 = 0; y2 < 32; y2++) {
              for (int z2 = 0; z2 < 32; z2++) {
                voxChunk(chunkData, x + x2, y + y2, z + z2) = value;
              }
            }
          }
        }
      }
    }
  }
}
uint8_t *VoxelWorld::getChunk(glm::vec3 position) {
  return voxelData[voxelChunkMapData[((int)position.z / 128) * 64 +
                                     ((int)position.y / 128) * 8 +
                                     ((int)position.x / 128)]]
      .data;
}

uint16_t VoxelWorld::getChunkID(glm::vec3 position) {
  return voxelChunkMapData[((int)position.z / 128) * 64 +
                           ((int)position.y / 128) * 8 +
                           ((int)position.x / 128)];
}

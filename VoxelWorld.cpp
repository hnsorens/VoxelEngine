#include "VoxelWorld.hpp"

VoxelWorld::VoxelWorld() {}
VoxelWorld::~VoxelWorld() {}
void VoxelWorld::generateTerrain() {}
void VoxelWorld::startWorkers(int numThreads) {}
void VoxelWorld::stopWorkers() {}
void VoxelWorld::requestChunk(uint16_t chunkID, float priority) {}
void VoxelWorld::sortChunks() {}
void VoxelWorld::updateVoxelChunkMap(int modValue, int offset) {}
void VoxelWorld::updateVoxels() {}
void VoxelWorld::chunkWorker() {}
void VoxelWorld::generateChunk(VoxelChunk& chunk) {}
uint8_t* VoxelWorld::getChunk(glm::vec3 position) { return nullptr; }
uint16_t VoxelWorld::getChunkID(glm::vec3 position) { return 0; } 
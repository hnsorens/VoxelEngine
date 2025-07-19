#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <cstdint>

class VoxelWorld {
public:
    struct VoxelChunk {
        uint8_t* data;
        glm::ivec3 position;
        bool inQueue;
    };

    VoxelWorld();
    ~VoxelWorld();

    void generateTerrain();
    void startWorkers(int numThreads);
    void stopWorkers();
    void requestChunk(uint16_t chunkID, float priority);
    void sortChunks();
    void updateVoxelChunkMap(int modValue, int offset);
    void updateVoxels();
    void chunkWorker();
    void generateChunk(VoxelChunk& chunk);
    uint8_t* getChunk(glm::vec3 position);
    uint16_t getChunkID(glm::vec3 position);

private:
    std::vector<VoxelChunk> voxelData;
    std::vector<std::thread> workers;
    std::vector<uint16_t> chunkQueue;
    std::unordered_set<uint16_t> activeChunks;
    std::mutex queueMutex;
    std::condition_variable queueCond;
    bool stopThreads;
}; 
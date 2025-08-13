#include "Engine.hpp"

int main() {
  try {
    VoxelEngine::run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
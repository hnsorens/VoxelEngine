# VoxelEngine

A high-performance, modern voxel engine written in C++ with Vulkan and GLFW.

## Features
- Real-time raytracing with Vulkan
- Procedural terrain generation
- Multi-threaded chunk management
- Modular, extensible architecture

## Project Structure
```
VoxelEngine/
├── src/         # C++ source files
├── include/     # C++ header files
├── shaders/     # GLSL shader sources
├── bin/         # Compiled binaries and SPIR-V shaders
├── Makefile     # Build system
├── LICENSE      # License file
└── README.md    # This file
```

## Dependencies
- Vulkan SDK
- GLFW
- GLM
- glslc (from Vulkan SDK, for shader compilation)
- g++ (C++17 or newer)

## Building
To build the engine and shaders:
```sh
make
```

This will:
- Compile all shaders in `shaders/` to SPIR-V in `bin/`
- Build the C++ source files in `src/`
- Output the binary to `bin/voxels`

## Running
To run the engine:
```sh
make run
```
Or manually:
```sh
./bin/voxels
```

## Cleaning
To remove all build artifacts:
```sh
make clean
```

## Adding New Shaders
- Place new `.glsl` files in `shaders/`.
- Add a line to the `Makefile` to compile it to `bin/yourshader.spv`.

## License
See [LICENSE](LICENSE) for details.

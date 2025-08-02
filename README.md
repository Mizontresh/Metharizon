# Metharizon

A Vulkan-based raymarching and physics engine for building the game Metharizon.

## Overview

Metharizon is a GPU-accelerated raymarching engine that processes both rendering and physics on the GPU with minimal CPU overhead. The engine is designed to be the foundation for a game that combines real-time raymarching with physics simulation.

## Features

- **Vulkan-based rendering**: Modern GPU-accelerated graphics pipeline
- **Raymarching on GPU**: Real-time distance field rendering
- **Physics simulation**: GPU-accelerated physics processing
- **Modular architecture**: Well-structured codebase with separate components
- **Cross-platform**: Windows, Linux, and macOS support (planned)

## Project Structure

```
metharizon/
├── src/
│   ├── main.cpp                 # Application entry point
│   ├── core/
│   │   ├── Logger.h/cpp         # Logging system
│   │   └── Window.h/cpp         # GLFW window management
│   ├── vulkan/
│   │   ├── VulkanApplication.h/cpp  # Main Vulkan application
│   │   ├── VulkanInstance.h/cpp     # Vulkan instance and validation
│   │   ├── VulkanDevice.h/cpp       # Physical and logical device
│   │   ├── VulkanSwapChain.h/cpp    # Swap chain management
│   │   ├── VulkanRenderPass.h/cpp   # Render pass creation
│   │   ├── VulkanPipeline.h/cpp     # Graphics pipeline
│   │   ├── VulkanCommandPool.h/cpp  # Command buffer management
│   │   └── VulkanDescriptorPool.h/cpp # Descriptor sets
│   ├── rendering/
│   │   └── RaymarchRenderer.h/cpp   # Raymarching renderer
│   └── physics/                     # Physics components (future)
├── shaders/
│   ├── raymarch.vert              # Vertex shader
│   └── raymarch.frag              # Fragment shader (raymarching)
├── CMakeLists.txt                 # Build configuration
└── README.md                      # This file
```

## Dependencies

- **Vulkan SDK**: Latest version (1.3+ recommended)
- **GLFW**: Window management and input
- **GLM**: Mathematics library
- **CMake**: Build system (3.16+)

## Building

### Prerequisites

1. **Install Vulkan SDK**:
   - Windows: Download from [LunarG](https://vulkan.lunarg.com/sdk/home)
   - Linux: `sudo apt install vulkan-tools vulkan-validationlayers`
   - macOS: Download from [LunarG](https://vulkan.lunarg.com/sdk/home)

2. **Install GLFW**:
   - Windows: `vcpkg install glfw3`
   - Linux: `sudo apt install libglfw3-dev`
   - macOS: `brew install glfw`

3. **Install GLM**:
   - Windows: `vcpkg install glm`
   - Linux: `sudo apt install libglm-dev`
   - macOS: `brew install glm`

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd metharizon
   ```

2. **Create build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```

4. **Build the project**:
   ```bash
   cmake --build .
   ```

5. **Run the application**:
   ```bash
   ./Metharizon
   ```

## Development

### Architecture

The application follows a modular architecture:

- **Core**: Basic utilities (logging, window management)
- **Vulkan**: Low-level Vulkan abstractions
- **Rendering**: High-level rendering components
- **Physics**: GPU-accelerated physics (future)

### Adding Features

1. **New Vulkan components**: Add to `src/vulkan/`
2. **Rendering features**: Add to `src/rendering/`
3. **Physics**: Add to `src/physics/`
4. **Shaders**: Add to `shaders/`

### Debugging

The application includes comprehensive logging and Vulkan validation layers. Check the console output for detailed information about initialization and runtime behavior.

## Roadmap

- [x] Basic Vulkan setup
- [x] Window management
- [x] Swap chain and rendering pipeline
- [x] Basic raymarching shader
- [ ] Advanced raymarching features
- [ ] Physics simulation on GPU
- [ ] Input handling
- [ ] Camera system
- [ ] Scene management
- [ ] Performance optimization

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

[Add your license here]

## Acknowledgments

- Vulkan Tutorial for the foundation
- GLFW for window management
- GLM for mathematics 
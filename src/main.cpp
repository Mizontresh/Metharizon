#include <iostream>
#include <stdexcept>
#include <memory>
#include "vulkan/VulkanApplication.h"
#include "core/Window.h"
#include "core/Logger.h"

int main() {
    try {
        Logger::init();
        Logger::info("Starting Metharizon - Vulkan Raymarching Physics Engine");
        
        // Create window
        auto window = std::make_unique<Window>("Metharizon", 1280, 720);
        
        // Create Vulkan application
        auto app = std::make_unique<VulkanApplication>(window.get());
        
        // Main loop
        while (!window->shouldClose()) {
            window->pollEvents();
            app->handleInput();
            app->render();
        }
        
        Logger::info("Shutting down Metharizon");
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
} 
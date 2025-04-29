// src/main.cpp

#include <iostream>
#include <cmath>        // for std::fmod

#include <SDL.h>

// Enable GLM’s experimental quaternion extensions
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>  // for glm::two_pi
#include <glm/gtx/quaternion.hpp> // quaternion × vec3

#include "Window.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include "Raymarcher.hpp"

int main(int argc, char* argv[]) {
    // 1) Window + OpenGL context
    Window window;
    if (!window.init(1280, 720, "Metharizon Engine")) {
        std::cerr << "Failed to open window\n";
        return -1;
    }
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // 2) Start the threaded timer
    Time time;
    time.init();

    // 3) Input system
    Input input;

    // 4) Raymarcher (compiles shaders & builds VAO)
    Raymarcher rm;
    if (!rm.init()) {
        std::cerr << "Failed to initialize Raymarcher\n";
        return -1;
    }

    // 5) Shader uniform config
    RaymarchConfig cfg;
    cfg.resolution = glm::vec2(1280.0f, 720.0f);
    cfg.maxSteps   = 64;
    cfg.epsilon    = 0.001f;
    cfg.pass       = 0;

    // 6) Camera state & parameters
    glm::vec3 camPos(0.0f, 0.0f, 3.0f);
    glm::quat orientation(1.0f, 0.0f, 0.0f, 0.0f);  // identity quaternion
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    const float speed       = 20.0f;    // movement units/sec
    const float sensitivity = 0.0025f;  // mouse sensitivity
    const float twoPi       = glm::two_pi<float>();

    // 7) Main loop
    while (window.isOpen()) {
        window.pollEvents();
        input.update();
        if (input.wasKeyPressed(SDL_SCANCODE_ESCAPE)) break;

        // a) Mouse‐look
        int dx=0, dy=0;
        input.getMouseDelta(dx, dy);

        // Yaw around camera’s local up
        glm::vec3 localUp = orientation * worldUp;
        glm::quat qYaw    = glm::angleAxis(-dx * sensitivity, localUp);
        orientation       = glm::normalize(qYaw * orientation);

        // Pitch around camera’s local right
        glm::vec3 localRight = orientation * glm::vec3(1,0,0);
        glm::quat qPitch     = glm::angleAxis(-dy * sensitivity, localRight);
        orientation          = glm::normalize(qPitch * orientation);

        // b) Recompute basis vectors
        glm::vec3 forward = orientation * glm::vec3(0, 0, -1);
        glm::vec3 right   = orientation * glm::vec3(1, 0,  0);
        glm::vec3 up      = orientation * glm::vec3(0, 1,  0);

        // c) Movement: WASD, and now 6-DOF along local up/down
        float dt = time.deltaTime();
        if (input.isKeyDown(SDL_SCANCODE_W))      camPos += forward * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_S))      camPos -= forward * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_A))      camPos -= right   * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_D))      camPos += right   * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_SPACE))  camPos += up      * speed * dt;  // ascend
        if (input.isKeyDown(SDL_SCANCODE_LSHIFT)) camPos -= up      * speed * dt;  // descend

        // d) Upload uniforms
        cfg.time       = time.totalTime();
        cfg.camPos     = camPos;
        cfg.camForward = forward;
        cfg.camRight   = right;
        cfg.camUp      = up;

        // e) Render
        window.clear();
        rm.render(cfg);
        window.swapBuffers();
    }

    // 8) Cleanup
    time.shutdown();
    return 0;
}

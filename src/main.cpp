#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include "Raymarcher.hpp"

int main() {
    Window window;
    if (!window.init(1280,720,"Metharizon Engine")) return -1;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    Time time; time.init();
    Input input;

    Raymarcher rm;
    if (!rm.init()) return -1;

    RaymarchConfig cfg;
    cfg.maxSteps = 64;
    cfg.epsilon  = 0.001f;
    cfg.pass     = 0;

    glm::vec3 camPos(0,0,3);
    glm::quat orientation(1,0,0,0);
    const glm::vec3 worldUp(0,1,0);
    const float speed=20.0f, sens=0.0025f;
    int mode=2;
    glm::mat4 objTransform(1.0f);

    // dynamic spawn data
    std::vector<glm::vec3> positions;
    std::vector<float>     minors;
    std::vector<unsigned>  ids;
    unsigned nextID = 1;
    const float spawnDist = 2.0f;
    const float minorR    = 0.2f;

    while (window.isOpen()) {
        window.pollEvents();
        input.update();

        float dt = time.deltaTime();
        float ang = glm::radians(90.0f) * dt;

        // Rotate unit‐sphere
        if(input.isKeyDown(SDL_SCANCODE_U)) objTransform = glm::rotate(objTransform, +ang, glm::vec3(1,0,0));
        if(input.isKeyDown(SDL_SCANCODE_H)) objTransform = glm::rotate(objTransform, -ang, glm::vec3(1,0,0));
        if(input.isKeyDown(SDL_SCANCODE_J)) objTransform = glm::rotate(objTransform, +ang, glm::vec3(0,1,0));
        if(input.isKeyDown(SDL_SCANCODE_K)) objTransform = glm::rotate(objTransform, -ang, glm::vec3(0,1,0));
        if(input.isKeyDown(SDL_SCANCODE_L)) objTransform = glm::rotate(objTransform, +ang, glm::vec3(0,0,1));
        if(input.isKeyDown(SDL_SCANCODE_O)) objTransform = glm::rotate(objTransform, -ang, glm::vec3(0,0,1));

        if(input.wasKeyPressed(SDL_SCANCODE_ESCAPE)) break;

        // Mouse‐look
        int mx, my; input.getMouseDelta(mx, my);
        glm::vec3 localUp = orientation * worldUp;
        orientation = glm::normalize(glm::angleAxis(-mx * sens, localUp) * orientation);
        orientation = glm::normalize(glm::angleAxis(-my * sens, orientation * glm::vec3(1,0,0)) * orientation);

        glm::vec3 forward = orientation * glm::vec3(0,0,-1);
        glm::vec3 right   = orientation * glm::vec3(1,0, 0);
        glm::vec3 upVec   = orientation * glm::vec3(0,1, 0);

        // Movement
        if(input.isKeyDown(SDL_SCANCODE_W))      camPos += forward * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_S))      camPos -= forward * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_A))      camPos -= right   * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_D))      camPos += right   * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_SPACE))  camPos += upVec   * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_LSHIFT)) camPos -= upVec   * speed * dt;

        // Spawn on 'P': unlimited
        if(input.wasKeyPressed(SDL_SCANCODE_P)) {
            glm::vec3 p = camPos + forward * spawnDist;
            positions.push_back(p);
            minors   .push_back(minorR);
            ids      .push_back(nextID++);
        }

        rm.updateSpawns(positions, minors, ids);

        // Update camera uniforms
        if (input.isKeyDown(SDL_SCANCODE_W))      camPos += forward * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_S))      camPos -= forward * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_A))      camPos -= right   * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_D))      camPos += right   * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_SPACE))  camPos += upVec   * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_LSHIFT)) camPos -= upVec   * speed * dt;

        int w,h; window.getSize(w,h);
        cfg.resolution = glm::vec2(float(w), float(h));
        cfg.time       = time.totalTime();
        cfg.camPos     = camPos;
        cfg.camForward = forward;
        cfg.camRight   = right;
        cfg.camUp      = upVec;

        // ** Restore FPS + ms in title **
        float fps = dt > 0.0f ? 1.0f / dt : 0.0f;
        float ms  = dt * 1000.0f;
        char title[128];
        std::snprintf(title, 128,
            "Metharizon | Mode %d | %.1f FPS | %.2f ms/frame | %u objects",
            mode, fps, ms, (unsigned)positions.size());
        window.setTitle(title);

        window.clear();
        glm::mat4 inv = glm::inverse(objTransform);
        rm.render(cfg, mode, inv);
        window.swapBuffers();
    }

    time.shutdown();
    return 0;
}

#include <iostream>
#include <cmath>
#include <cstdio>
#include <string>
#include <SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include "Raymarcher.hpp"

int main(int argc, char* argv[]) {
    Window window;
    if (!window.init(1280, 720, "Metharizon Engine")) return -1;
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
    const float speed = 20.0f, sensitivity=0.0025f;

    int mode = 2;
    glm::mat4 objTransform = glm::mat4(1.0f);

    while (window.isOpen()) {
        window.pollEvents();
        input.update();

        // rotate object:
        float dt = time.deltaTime();
        float angle = glm::radians(90.0f) * dt; // 90°/s

        if (input.isKeyDown(SDL_SCANCODE_U))
            objTransform = glm::rotate(objTransform, +angle, glm::vec3(1,0,0));
        if (input.isKeyDown(SDL_SCANCODE_H))
            objTransform = glm::rotate(objTransform, -angle, glm::vec3(1,0,0));
        if (input.isKeyDown(SDL_SCANCODE_J))
            objTransform = glm::rotate(objTransform, +angle, glm::vec3(0,1,0));
        if (input.isKeyDown(SDL_SCANCODE_K))
            objTransform = glm::rotate(objTransform, -angle, glm::vec3(0,1,0));
        if (input.isKeyDown(SDL_SCANCODE_L))
            objTransform = glm::rotate(objTransform, +angle, glm::vec3(0,0,1));
        if (input.isKeyDown(SDL_SCANCODE_O))
            objTransform = glm::rotate(objTransform, -angle, glm::vec3(0,0,1));

        // camera look + movement
        if (input.wasKeyPressed(SDL_SCANCODE_ESCAPE)) break;
        int dx,dy; input.getMouseDelta(dx,dy);
        glm::vec3 localUp = orientation * worldUp;
        orientation = glm::normalize(glm::angleAxis(-dx*sensitivity,localUp)*orientation);
        orientation = glm::normalize(glm::angleAxis(-dy*sensitivity,orientation*glm::vec3(1,0,0))*orientation);

        glm::vec3 forward = orientation * glm::vec3(0,0,-1);
        glm::vec3 right   = orientation * glm::vec3(1,0, 0);
        glm::vec3 upVec   = orientation * glm::vec3(0,1, 0);

        if (input.isKeyDown(SDL_SCANCODE_W))      camPos += forward * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_S))      camPos -= forward * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_A))      camPos -= right  * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_D))      camPos += right  * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_SPACE))  camPos += upVec  * speed * dt;
        if (input.isKeyDown(SDL_SCANCODE_LSHIFT)) camPos -= upVec  * speed * dt;

        // resolution + FPS title
        int w,h; window.getSize(w,h);
        cfg.resolution = glm::vec2(float(w), float(h));
        cfg.time       = time.totalTime();
        cfg.camPos     = camPos;
        cfg.camForward = forward;
        cfg.camRight   = right;
        cfg.camUp      = upVec;

        static const char* names[3]={"Basic","Relaxed","Enhanced"};
        float fps = dt>0?1.0f/dt:0;
        char title[128];
        std::snprintf(title,128,"Metharizon — %s | %.1f FPS",names[mode],fps);
        window.setTitle(title);

        window.clear();
        // pass inverse transform to shader
        glm::mat4 inv = glm::inverse(objTransform);
        rm.render(cfg, mode, inv);
        window.swapBuffers();
    }

    time.shutdown();
    return 0;
}

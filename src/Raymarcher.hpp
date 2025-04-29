#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct RaymarchConfig {
    glm::vec2 resolution;   // in pixels
    float     time;         
    int       maxSteps;     
    float     epsilon;      
    int       pass;         

    // New camera parameters:
    glm::vec3 camPos;
    glm::vec3 camForward;
    glm::vec3 camRight;
    glm::vec3 camUp;
};

class Raymarcher {
public:
    Raymarcher();
    ~Raymarcher();

    bool init();                      // compile & link shaders, build VAO
    void render(const RaymarchConfig& cfg);

private:
    GLuint loadShader(const char* path, GLenum type);
    bool   linkProgram(GLuint vs, GLuint fs);
    void   buildFullScreenTriangle();

    GLuint _program       = 0;
    GLint  _locConfig     = -1;
    GLint  _locCamPos     = -1;
    GLint  _locCamForward = -1;
    GLint  _locCamRight   = -1;
    GLint  _locCamUp      = -1;
    GLuint _vao           = 0;
};

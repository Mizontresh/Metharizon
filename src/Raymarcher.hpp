#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

struct RaymarchConfig {
    glm::vec2 resolution;
    float     time;
    int       maxSteps;
    float     epsilon;
    int       pass;
    glm::vec3 camPos, camForward, camRight, camUp;
};

class Raymarcher {
public:
    Raymarcher();
    ~Raymarcher();

    bool init();

    // mode: 0=basic,1=relaxed,2=enhanced
    // objInv: inverse model matrix for the one object
    void render(const RaymarchConfig& cfg, int mode, const glm::mat4& objInv);

private:
    GLuint loadShader(const char* path, GLenum type);
    bool   linkProgram(GLuint vs, GLuint fs);
    void   buildFullScreenTriangle();

    GLuint _program = 0, _vao = 0;
    // uniforms
    GLint _locResolution, _locTime, _locMaxSteps, _locEpsilon, _locPass;
    GLint _locMode;
    GLint _locObjInv;       // <— new
    GLint _locCamPos, _locCamForward, _locCamRight, _locCamUp;
};

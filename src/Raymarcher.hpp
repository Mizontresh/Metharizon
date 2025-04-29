#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

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
    void render(const RaymarchConfig& cfg, int mode, const glm::mat4& objInv);

    // new: receive dynamic spawn lists and their unique IDs
    void updateSpawns(const std::vector<glm::vec3>& positions,
                      const std::vector<float>&     minors,
                      const std::vector<unsigned>&   ids);

private:
    GLuint loadShader(const char* path, GLenum type);
    bool   linkProgram(GLuint vs, GLuint fs);
    void   buildFullScreenTriangle();

    GLuint _program = 0, _vao = 0;

    // existing uniforms
    GLint _locResolution, _locTime, _locMaxSteps, _locEpsilon, _locPass;
    GLint _locMode, _locObjInv;
    GLint _locCamPos, _locCamForward, _locCamRight, _locCamUp;
    GLint _locSpawnCount;

    // SSBOs for unlimited spawns + IDs
    GLuint _ssboPosMinor = 0;
    GLuint _ssboIDs      = 0;

    // CPU‐side buffers
    std::vector<glm::vec4> spawnPosMin;  // (x,y,z)=pos, w=minor radius
    std::vector<unsigned>  spawnIDs;
};

// Raymarcher.hpp
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
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

    // Initialize GL program, get uniforms, create SSBOs, build VAO
    bool init();

    // Render full-screen triangle; reads SSBOs and uniforms
    void render(const RaymarchConfig& cfg, int mode, const glm::mat4& objInv);

    // Upload dynamic spawn lists: positions, radii, IDs, orientations
    void updateSpawns(const std::vector<glm::vec3>& positions,
                      const std::vector<float>&     minors,
                      const std::vector<unsigned>&   ids,
                      const std::vector<glm::quat>&  orientations);

private:
    // Helpers for shader loading/linking & VAO setup
    GLuint loadShader(const char* path, GLenum type);
    bool   linkProgram(GLuint vs, GLuint fs);
    void   buildFullScreenTriangle();

    // GL handles
    GLuint _program = 0;
    GLuint _vao     = 0;
    GLuint _ssboPosMinor = 0;
    GLuint _ssboIDs      = 0;
    GLuint _ssboOrient   = 0;

    // Uniform locations
    GLint _locResolution, _locTime, _locMaxSteps, _locEpsilon, _locPass;
    GLint _locMode, _locObjInv;
    GLint _locCamPos, _locCamForward, _locCamRight, _locCamUp;
    GLint _locSpawnCount;

    // CPU‐side staging buffers
    std::vector<glm::vec4> spawnPosMin;  // xyz = pos, w = radius
    std::vector<unsigned>  spawnIDs;
    std::vector<glm::quat> spawnOrient;  // quaternion per instance
};

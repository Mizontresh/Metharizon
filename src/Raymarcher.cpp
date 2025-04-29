#include "Raymarcher.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const char* path) {
    std::ifstream in(path);
    std::stringstream buf;
    buf << in.rdbuf();
    return buf.str();
}

Raymarcher::Raymarcher() {}
Raymarcher::~Raymarcher() {
    if (_program)       glDeleteProgram(_program);
    if (_vao)           glDeleteVertexArrays(1, &_vao);
    if (_ssboPosMinor)  glDeleteBuffers(1, &_ssboPosMinor);
    if (_ssboIDs)       glDeleteBuffers(1, &_ssboIDs);
}

bool Raymarcher::init() {
    GLuint vs = loadShader("shaders/fullscreen.vert", GL_VERTEX_SHADER);
    GLuint fs = loadShader("shaders/raymarch.frag",    GL_FRAGMENT_SHADER);
    if (!vs || !fs) return false;
    if (!linkProgram(vs, fs)) return false;
    glDeleteShader(vs); glDeleteShader(fs);

    // fetch uniforms
    _locResolution  = glGetUniformLocation(_program, "u_resolution");
    _locTime        = glGetUniformLocation(_program, "u_time");
    _locMaxSteps    = glGetUniformLocation(_program, "u_maxSteps");
    _locEpsilon     = glGetUniformLocation(_program, "u_epsilon");
    _locPass        = glGetUniformLocation(_program, "u_pass");
    _locMode        = glGetUniformLocation(_program, "u_mode");
    _locObjInv      = glGetUniformLocation(_program, "u_objInvTransform");
    _locCamPos      = glGetUniformLocation(_program, "u_camPos");
    _locCamForward  = glGetUniformLocation(_program, "u_camForward");
    _locCamRight    = glGetUniformLocation(_program, "u_camRight");
    _locCamUp       = glGetUniformLocation(_program, "u_camUp");
    _locSpawnCount  = glGetUniformLocation(_program, "u_spawnCount");

    // create SSBOs
    glGenBuffers(1, &_ssboPosMinor);
    glGenBuffers(1, &_ssboIDs);

    buildFullScreenTriangle();
    return true;
}

void Raymarcher::updateSpawns(const std::vector<glm::vec3>& positions,
                              const std::vector<float>&     minors,
                              const std::vector<unsigned>&   ids)
{
    size_t n = positions.size();
    spawnPosMin.resize(n);
    spawnIDs.resize(n);
    for (size_t i = 0; i < n; ++i) {
        spawnPosMin[i] = glm::vec4(positions[i], minors[i]);
        spawnIDs[i]    = ids[i];
    }
}

void Raymarcher::render(const RaymarchConfig& cfg, int mode, const glm::mat4& objInv) {
    glUseProgram(_program);

    // camera + mode
    glUniform1i (_locMode,       mode);
    glUniform2fv(_locResolution, 1, &cfg.resolution[0]);
    glUniform1f (_locTime,       cfg.time);
    glUniform1i (_locMaxSteps,   cfg.maxSteps);
    glUniform1f (_locEpsilon,    cfg.epsilon);
    glUniform1i (_locPass,       cfg.pass);
    glUniformMatrix4fv(_locObjInv, 1, GL_FALSE, &objInv[0][0]);
    glUniform3fv(_locCamPos,     1, &cfg.camPos[0]);
    glUniform3fv(_locCamForward, 1, &cfg.camForward[0]);
    glUniform3fv(_locCamRight,   1, &cfg.camRight[0]);
    glUniform3fv(_locCamUp,      1, &cfg.camUp[0]);

    // upload spawn count
    unsigned count = (unsigned)spawnPosMin.size();
    glUniform1ui(_locSpawnCount, count);

    // SSBO: positions+minors
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssboPosMinor);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 count * sizeof(glm::vec4),
                 spawnPosMin.data(),
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _ssboPosMinor);

    // SSBO: IDs
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssboIDs);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 count * sizeof(unsigned),
                 spawnIDs.data(),
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _ssboIDs);

    // draw fullscreen triangle
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

// loadShader, linkProgram, buildFullScreenTriangle unchanged…

GLuint Raymarcher::loadShader(const char* path, GLenum type) {
    auto src = readFile(path);
    const char* ptr = src.c_str();
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &ptr, nullptr);
    glCompileShader(s);
    GLint ok = GL_FALSE;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, ' ');
        glGetShaderInfoLog(s, len, nullptr, &log[0]);
        std::cerr << "Compile error in " << path << ":\n" << log;
        glDeleteShader(s);
        return 0;
    }
    return s;
}

bool Raymarcher::linkProgram(GLuint vs, GLuint fs) {
    _program = glCreateProgram();
    glAttachShader(_program, vs);
    glAttachShader(_program, fs);
    glLinkProgram(_program);
    GLint ok = GL_FALSE;
    glGetProgramiv(_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len; glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, ' ');
        glGetProgramInfoLog(_program, len, nullptr, &log[0]);
        std::cerr << "Link error:\n" << log;
        glDeleteProgram(_program);
        _program = 0;
        return false;
    }
    return true;
}

void Raymarcher::buildFullScreenTriangle() {
    static const float verts[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f
    };
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
}

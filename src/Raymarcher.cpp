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
    if (_program) glDeleteProgram(_program);
    if (_vao)     glDeleteVertexArrays(1, &_vao);
}

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

bool Raymarcher::init() {
    GLuint vs = loadShader("shaders/fullscreen.vert", GL_VERTEX_SHADER);
    GLuint fs = loadShader("shaders/raymarch.frag",    GL_FRAGMENT_SHADER);
    if (!vs || !fs) return false;
    if (!linkProgram(vs, fs)) return false;
    glDeleteShader(vs);
    glDeleteShader(fs);

    _locConfig     = glGetUniformLocation(_program, "u_config");
    _locCamPos     = glGetUniformLocation(_program, "u_camPos");
    _locCamForward = glGetUniformLocation(_program, "u_camForward");
    _locCamRight   = glGetUniformLocation(_program, "u_camRight");
    _locCamUp      = glGetUniformLocation(_program, "u_camUp");

    buildFullScreenTriangle();
    return true;
}

void Raymarcher::render(const RaymarchConfig& cfg) {
    glUseProgram(_program);

    // upload config array
    float data[6] = {
        cfg.resolution.x,      // [0]
        cfg.resolution.y,      // [1]
        cfg.time,              // [2]
        float(cfg.maxSteps),   // [3]
        cfg.epsilon,           // [4]
        float(cfg.pass)        // [5]
    };
    glUniform1fv(_locConfig, 6, data);

    // upload camera
    glUniform3fv(_locCamPos,     1, &cfg.camPos[0]);
    glUniform3fv(_locCamForward, 1, &cfg.camForward[0]);
    glUniform3fv(_locCamRight,   1, &cfg.camRight[0]);
    glUniform3fv(_locCamUp,      1, &cfg.camUp[0]);

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

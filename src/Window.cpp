#include "Window.hpp"
#include <iostream>
#include <glad/glad.h>  // for glViewport etc.
#include <SDL.h>        // for SDL_* types and functions

Window::Window()
  : window_(nullptr)
  , glContext_(nullptr)
  , open_(false)
  , dragging_(false)
  , dragOffsetX_(0)
  , dragOffsetY_(0)
{}

Window::~Window() {
    if (glContext_) SDL_GL_DeleteContext(glContext_);
    if (window_)    SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool Window::init(int width, int height, const char* title) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window_ = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );
    if (!window_) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return false;
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << "\n";
        return false;
    }

    // initialize GLAD (must be done *after* creating the GL context)
    if (!gladLoadGLLoader((SDL_GL_GetProcAddress))) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    // vsync on
    SDL_GL_SetSwapInterval(1);

    // set initial viewport to match window size
    glViewport(0, 0, width, height);

    open_ = true;
    return true;
}

void Window::pollEvents() {
    SDL_Event e;
    int w, h;
    SDL_GetWindowSize(window_, &w, &h);

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
          case SDL_QUIT:
            open_ = false;
            break;

          case SDL_KEYDOWN:
            if (e.key.keysym.scancode == SDL_SCANCODE_F11) {
                toggleFullscreen();
                // after toggling, fetch new size and reset viewport
                SDL_GetWindowSize(window_, &w, &h);
                glViewport(0, 0, w, h);
            }
            break;

          case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                // window was resized (or toggled fullscreen)
                glViewport(0, 0, e.window.data1, e.window.data2);
            }
            break;

          case SDL_MOUSEBUTTONDOWN:
            if (e.button.button == SDL_BUTTON_LEFT) {
                int x = e.button.x, y = e.button.y;
                if (y < TITLEBAR_HEIGHT && x > w - CLOSE_BUTTON_SIZE) {
                    open_ = false;
                } else if (y < TITLEBAR_HEIGHT) {
                    int mx, my, wx, wy;
                    SDL_GetGlobalMouseState(&mx, &my);
                    SDL_GetWindowPosition(window_, &wx, &wy);
                    dragOffsetX_ = mx - wx;
                    dragOffsetY_ = my - wy;
                    dragging_    = true;
                }
            }
            break;

          case SDL_MOUSEBUTTONUP:
            if (e.button.button == SDL_BUTTON_LEFT)
                dragging_ = false;
            break;

          case SDL_MOUSEMOTION:
            if (dragging_) {
                int mx, my;
                SDL_GetGlobalMouseState(&mx, &my);
                SDL_SetWindowPosition(window_,
                    mx - dragOffsetX_, my - dragOffsetY_);
            }
            break;

          default:
            break;
        }
    }
}

void Window::clear() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::swapBuffers() {
    SDL_GL_SwapWindow(window_);
}

void Window::getSize(int& width, int& height) const {
    SDL_GetWindowSize(window_, &width, &height);
}

void Window::setTitle(const std::string& title) {
    SDL_SetWindowTitle(window_, title.c_str());
}

void Window::toggleFullscreen() {
    Uint32 flags = SDL_GetWindowFlags(window_);
    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_SetWindowFullscreen(window_, 0);
    } else {
        SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

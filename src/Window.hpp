#pragma once

#include <SDL.h>
#include <glad/glad.h>

class Window {
public:
    Window();
    ~Window();

    bool init(int width, int height, const char* title);
    void pollEvents();
    bool isOpen() const { return open_; }
    void clear();
    void swapBuffers();

private:
    SDL_Window*   window_    = nullptr;
    SDL_GLContext glContext_ = nullptr;
    bool          open_      = false;

    // Custom drag
    bool dragging_    = false;
    int  dragOffsetX_ = 0;
    int  dragOffsetY_ = 0;
    static constexpr int TITLEBAR_HEIGHT   = 30;
    static constexpr int CLOSE_BUTTON_SIZE = 30;
};

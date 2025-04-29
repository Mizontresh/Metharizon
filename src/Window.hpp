#pragma once
#include <SDL.h>
#include <string>

class Window {
public:
    Window();
    ~Window();

    bool init(int width, int height, const char* title);
    void pollEvents();
    void clear();
    void swapBuffers();

    // Query current window size
    void getSize(int& width, int& height) const;
    // Update the window title
    void setTitle(const std::string& title);

    // NEW: toggle between windowed <-> fullscreen
    void toggleFullscreen();

    bool isOpen() const { return open_; }

private:
    SDL_Window*   window_    = nullptr;
    SDL_GLContext glContext_ = nullptr;
    bool          open_      = false;
    bool          dragging_  = false;
    int           dragOffsetX_ = 0, dragOffsetY_ = 0;

    static constexpr int TITLEBAR_HEIGHT   = 30;
    static constexpr int CLOSE_BUTTON_SIZE = 30;
};

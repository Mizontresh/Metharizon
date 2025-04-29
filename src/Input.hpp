#pragma once
#include <SDL.h>
#include <array>
#include <algorithm>

class Input {
public:
    static constexpr int KEY_COUNT = SDL_NUM_SCANCODES;

    // Call once per frame
    void update();

    // Keyboard queries
    bool isKeyDown    (SDL_Scancode k) const { return curr_[k]; }
    bool wasKeyPressed(SDL_Scancode k) const { return curr_[k] && !prev_[k]; }
    bool wasKeyReleased(SDL_Scancode k) const { return !curr_[k] && prev_[k]; }

    // Mouse‐delta since last update (relative)
    void getMouseDelta(int &dx, int &dy) const {
        dx = mouseDX_; 
        dy = mouseDY_;
    }

private:
    std::array<Uint8, KEY_COUNT> prev_{}, curr_{};
    int mouseDX_ = 0, mouseDY_ = 0;
};

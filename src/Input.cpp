#include "Input.hpp"

void Input::update() {
    // 1) Keyboard
    prev_ = curr_;
    int n = 0;
    const Uint8* state = SDL_GetKeyboardState(&n);
    int copyN = std::min((int)KEY_COUNT, n);
    std::copy(state, state + copyN, curr_.begin());
    if (copyN < KEY_COUNT) {
        std::fill(curr_.begin() + copyN, curr_.end(), 0);
    }

    // 2) Mouse (relative motion)
    // This returns dx,dy since the last call, even in relative mode.
    SDL_GetRelativeMouseState(&mouseDX_, &mouseDY_);
}

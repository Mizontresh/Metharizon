#include "Time.hpp"
#include <thread>

void Time::init() {
    start_   = last_ = std::chrono::high_resolution_clock::now();
    running_.store(true);
    thread_ = std::thread([this]{ run(); });
}

void Time::shutdown() {
    running_.store(false);
    if (thread_.joinable()) thread_.join();
}

void Time::run() {
    while (running_.load()) {
        auto now = std::chrono::high_resolution_clock::now();
        float d = std::chrono::duration<float>(now - last_).count();
        delta_.store(d);
        float t = std::chrono::duration<float>(now - start_).count();
        total_.store(t);
        last_ = now;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

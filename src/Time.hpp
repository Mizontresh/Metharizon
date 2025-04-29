#pragma once
#include <atomic>
#include <thread>
#include <chrono>

class Time {
public:
    void init();
    void shutdown();
    float deltaTime() const { return delta_.load(); }
    float totalTime() const { return total_.load(); }

private:
    void run();
    std::atomic<float> delta_{0.0f}, total_{0.0f};
    std::atomic<bool>  running_{false};
    std::thread        thread_;
    std::chrono::high_resolution_clock::time_point last_, start_;
};

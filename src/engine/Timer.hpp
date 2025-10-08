#pragma once

#include <cstdint>
#include <queue>

namespace ast {

class Timer {
public:
    explicit Timer(int targetFps = 60);
    void startFrame();
    void endFrame();
    int getTargetFps() const { return targetFps_; }
    void setTargetFps(int targetFps) { targetFps_ = targetFps; }
    double getFps() const { return averageFps_; }
    double getDeltaTime() const;
    void reset();

    static double getClockTime();
    static double getRunningTime();
    static void resetClockTime();

private:
    inline static double initTime_{};
    static constexpr size_t frameSecSamples_s = 100;

    std::queue<double> frameSecHistory_;
    uint64_t startFrameTime_{};
    uint64_t previousFrameTime_{};
    uint64_t lastFpsUpdateTime_{};
    double targetFrameSec_{};
    double frameSecSum_{};
    double smoothedFrameSec_{};
    double averageFps_{};
    double sleepError_{};
    int targetFps_;
    int frames_{};
    int frameSecIndex_{};
};

}  // namespace ast

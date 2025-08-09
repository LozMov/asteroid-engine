#include "Timer.hpp"

namespace ast {

Timer::Timer(int target_fps)
    : targetFps_(target_fps),
      targetFrameSec_(1.0 / target_fps),
      startFrameTime_(SDL_GetPerformanceCounter()),
      lastFpsUpdateTime_(startFrameTime_),
      previousFrameTime_(startFrameTime_),
      smoothedFrameSec_(targetFrameSec_) {}

void Timer::startFrame() {
    previousFrameTime_ = startFrameTime_;
    startFrameTime_ = SDL_GetPerformanceCounter();
    ++frames_;

    // Calculate the frame time for the current frame
    double frame_sec =
        static_cast<double>(startFrameTime_ - previousFrameTime_) / SDL_GetPerformanceFrequency();
#ifdef AST_TIMER_FRAME_HISTORY
    frameSecHistory_.push(frame_sec);
    frameSecSum_ += frame_sec;
    if (frameSecHistory_.size() > frameSecSamples_s) {
        frameSecSum_ -= frameSecHistory_.front();
        frameSecHistory_.pop();
    }
#else
    smoothedFrameSec_ = 0.1 * frame_sec + 0.9 * smoothedFrameSec_;
#endif

    // Update FPS every second
    double sec_since_last_update =
        static_cast<double>(startFrameTime_ - lastFpsUpdateTime_) / SDL_GetPerformanceFrequency();
    if (sec_since_last_update >= 1.0) {
        averageFps_ = frames_ / sec_since_last_update;
        frames_ = 0;
        lastFpsUpdateTime_ = startFrameTime_;
    }
}

void Timer::endFrame() {
    uint64_t current_time = SDL_GetPerformanceCounter();
    // Remaining time = target frame time - elapsed time - sleep error
    double remaining_sec =
        targetFrameSec_ -
        static_cast<double>(current_time - startFrameTime_) / SDL_GetPerformanceFrequency() -
        sleepError_;
    if (remaining_sec > 0) {
        if (remaining_sec > 0.001) {
            // Sleep for 90% of the remaining time
            SDL_Delay(static_cast<uint32_t>(remaining_sec * 0.9 * 1000));
        }
        // Busy wait for the remaining time
        while (static_cast<double>(SDL_GetPerformanceCounter() - startFrameTime_) /
                   SDL_GetPerformanceFrequency() <
               targetFrameSec_) {
            SDL_Delay(0);
        }
        // Update sleep error
        double actual_sleep_sec = static_cast<double>(SDL_GetPerformanceCounter() - current_time) /
                                  SDL_GetPerformanceFrequency();
        sleepError_ = (remaining_sec - actual_sleep_sec) * 0.5;
    } else {
        sleepError_ = 0.0;  // Reset sleep error if overslept
    }
}

double Timer::getDeltaTime() const {
#ifdef AST_TIMER_FRAME_HISTORY
    return frameSecSum_ / frameSecHistory_.size();
#else
    return smoothedFrameSec_;
#endif
}

void Timer::reset() {
    frames_ = 0;
    startFrameTime_ = SDL_GetPerformanceCounter();
    previousFrameTime_ = startFrameTime_;
    smoothedFrameSec_ = targetFrameSec_;
    sleepError_ = 0.0;
    averageFps_ = 0.0;
    lastFpsUpdateTime_ = startFrameTime_;
}

double Timer::getClockTime() { return (SDL_GetTicks() - initTime_) / 1000.0; }

double Timer::getRunningTime() { return SDL_GetTicks() / 1000.0; }

void Timer::resetClockTime() { initTime_ = SDL_GetTicks(); }

}  // namespace ast
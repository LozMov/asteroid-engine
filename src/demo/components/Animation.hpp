#pragma once

// #include <vector>

#include "engine/ecs/Component.hpp"

namespace astd::components {

struct Animation : ast::Component {
    Animation() = default;
    Animation(float frameDuration, int frameStartIndex = 0, int frameCount = -1, bool loop = true,
              bool playing = true)
        : frameDuration(frameDuration),
          frameStartIndex(frameStartIndex),
          frameCount(frameCount),
          loop(loop),
          playing(playing) {}

    // std::vector<std::vector<int>> sequences = CHARACTER_SEQUENCES;
    int frameCount = -1;      // Number of frames in the current sequence
    int frameStartIndex = 0;  // Start index of the current sequence
    float frameDuration = 0.0f;
    float frameTimeElapsed = 0.0f;  // Set by AnimationSystem
    bool loop = true;
    bool playing = true;

    // inline static const std::vector<std::vector<int>> CHARACTER_SEQUENCES = {
    //     {0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};
};

}  // namespace astd::components
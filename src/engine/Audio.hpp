#pragma once

#include <SDL3/SDL_audio.h>

#include <string>
#include <unordered_map>

namespace ast {

class Audio {
public:
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    static Audio& getInstance();

    bool init();
    void shutdown();

    bool loadSound(const std::string& name, const std::string& filePath);
    void playSound(const std::string& name, int loops = 1);
    void stopSound(const std::string& name);

    float getMasterGain() const;
    void setMasterGain(float gain);
    float getSoundGain(const std::string& name) const;
    void setSoundGain(const std::string& name, float gain);
    bool isSoundPlaying(const std::string& name) const;
    void setAssetsDirectory(const std::string& directory);

    void update();

private:
    Audio() = default;

    struct AudioData {
        SDL_AudioSpec spec;
        Uint8* buffer = nullptr;
        SDL_AudioStream* stream = nullptr;
        Uint32 length = 0;
        Uint32 currentPosition = 0;
    };

    std::unordered_map<std::string, AudioData> sounds_;
    std::unordered_map<std::string, int> playing_;
    std::string assetsDirectory_;
    SDL_AudioDeviceID device_ = 0;
    bool initialized_ = false;
};

}  // namespace ast
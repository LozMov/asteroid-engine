#include "Audio.hpp"

#define MINIMP3_ONLY_MP3
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"

namespace ast {

Audio& Audio::getInstance() {
    static Audio instance;
    return instance;
}

bool Audio::init() {
    if (initialized_) {
        AST_WARN("Audio already initialized");
        return true;
    }
    AST_INFO("Initializing Audio...");
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_ERROR();
        return false;
    }
    device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!device_) {
        SDL_ERROR();
        return false;
    }
    initialized_ = true;
    return true;
}

void Audio::shutdown() {
    if (!initialized_) {
        AST_WARN("Audio not initialized, nothing to shutdown");
        return;
    }
    for (auto& [name, data] : sounds_) {
        SDL_free(data.buffer);
        SDL_DestroyAudioStream(data.stream);
    }
    sounds_.clear();
    playing_.clear();
    if (device_) {
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    initialized_ = false;
}

bool Audio::loadSound(const std::string& name, const std::string& filePath) {
    if (!initialized_) {
        AST_WARN("Audio not initialized, cannot load sound '{}'", name);
        return false;
    }
    if (sounds_.find(name) != sounds_.end()) {
        AST_WARN("Sound '{}' already loaded", name);
        return true;
    }

    AudioData data;
    if (!SDL_LoadWAV(filePath.c_str(), &data.spec, &data.buffer, &data.length)) {
        SDL_ERROR();
        return false;
    }
    SDL_AudioSpec deviceSpec;
    SDL_GetAudioDeviceFormat(device_, &deviceSpec, nullptr);
    data.stream = SDL_CreateAudioStream(&data.spec, &deviceSpec);
    if (!data.stream) {
        SDL_ERROR();
        SDL_free(data.buffer);
        return false;
    }
    // Bind the stream to the device
    if (!SDL_BindAudioStream(device_, data.stream)) {
        SDL_ERROR();
        SDL_free(data.buffer);
        SDL_DestroyAudioStream(data.stream);
        return false;
    }

    AST_INFO("Loaded sound '{}' from {} (length: {} bytes)", name, filePath, data.length);
    sounds_.emplace(name, std::move(data));
    return true;
}

void Audio::playSound(const std::string& name, int loops) {
    if (!initialized_) {
        AST_WARN("Audio not initialized, cannot play sound '{}'", name);
        return;
    }
    if (sounds_.find(name) == sounds_.end()) {
        if (!loadSound(name, assetsDirectory_ + "/" + name)) {
            return;
        }
    }

    AST_INFO("Playing sound '{}'", name);
    playing_[name] = loops;
}

void Audio::stopSound(const std::string& name) {
    if (!initialized_) {
        AST_WARN("Audio not initialized, cannot stop sound '{}'", name);
        return;
    }
    auto it = playing_.find(name);
    if (it != playing_.end()) {
        AST_INFO("Stopping sound '{}'", name);
        playing_.erase(it);
        sounds_[name].currentPosition = 0;
    } else {
        AST_WARN("Sound '{}' not playing", name);
    }
}

float Audio::getMasterGain() const {
    if (!initialized_) {
        AST_WARN("Audio not initialized, cannot get gain");
        return -1.0f;
    }
    return SDL_GetAudioDeviceGain(device_);
}

void Audio::setMasterGain(float gain) {
    if (!initialized_) {
        AST_WARN("Audio not initialized, cannot set gain");
        return;
    }
    AST_INFO("Setting master gain to {}", gain);
    SDL_SetAudioDeviceGain(device_, gain);
}

float Audio::getSoundGain(const std::string& name) const {
    auto it = sounds_.find(name);
    if (it != sounds_.end()) {
        return SDL_GetAudioStreamGain(it->second.stream);
    }
    AST_WARN("Sound '{}' not found", name);
    return -1.0f;  // SDL_GetAudioStreamGain() returns -1.0f on error
}

void Audio::setSoundGain(const std::string& name, float gain) {
    auto it = sounds_.find(name);
    if (it != sounds_.end()) {
        SDL_SetAudioStreamGain(it->second.stream, gain);
        AST_INFO("Set gain for sound '{}' to {}", name, gain);
    } else {
        AST_WARN("Sound '{}' not found", name);
    }
}

bool Audio::isSoundPlaying(const std::string& name) const {
    return playing_.find(name) != playing_.end();
}

void Audio::setAssetsDirectory(const std::string& directory) {
    assetsDirectory_ = directory;
}

void Audio::update() {
    constexpr Uint32 CHUNK_SIZE = 8192;  // 8KB chunks

    for (auto it = playing_.begin(); it != playing_.end();) {
        AudioData& data = sounds_[it->first];
        // Check if we need to feed more data to the stream
        if (SDL_GetAudioStreamQueued(data.stream) < CHUNK_SIZE) {
            // Calculate how much data we can feed
            int bytesToFeed = std::min(CHUNK_SIZE, data.length - data.currentPosition);
            if (bytesToFeed > 0) {
                SDL_PutAudioStreamData(data.stream, data.buffer + data.currentPosition,
                                       bytesToFeed);
                data.currentPosition += bytesToFeed;
                ++it;
            } else {
                // Finished playing, reset position
                data.currentPosition = 0;
                if (it->second != 0 && --it->second == 0) {
                    // No more loops, remove from playing
                    it = playing_.erase(it);
                } else {
                    ++it;
                }
            }
        } else {
            ++it;
        }
    }
}

}  // namespace ast
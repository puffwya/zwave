#pragma once

#include "../third_party/miniaudio.h"
#include <string>
#include <unordered_map>

class AudioManager
{
public:
    bool init();
    void shutdown();

    // Music
    void playMusic(const std::string& path, bool loop = true);
    void stopMusic();

    // Sound Effects
    void loadSFX(const std::string& name, const std::string& path);
    void playSFX(const std::string& name, float volume = 1.0f);

private:
    ma_engine engine{};

    // Music
    ma_sound music{};
    bool musicLoaded = false;

    bool initialized = false;

    // Sound effects
    std::unordered_map<std::string, ma_sound> sfx;
};


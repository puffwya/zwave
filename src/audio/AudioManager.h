#pragma once

#include "../third_party/miniaudio.h"
#include <string>

class AudioManager
{
public:
    bool init();
    void shutdown();

    void playMusic(const std::string& path, bool loop = true);
    void stopMusic();

private:
    ma_engine engine{};
    ma_sound music{};
    bool musicLoaded = false;
};


#define MINIAUDIO_IMPLEMENTATION
#include "../third_party/miniaudio.h"

#include "AudioManager.h"
#include <iostream>

bool AudioManager::init()
{
    if (ma_engine_init(nullptr, &engine) != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio engine\n";
        return false;
    }
    return true;
}

void AudioManager::playMusic(const std::string& path, bool loop)
{
    if (musicLoaded)
    {
        ma_sound_stop(&music);
        ma_sound_uninit(&music);
        musicLoaded = false;
    }

    if (ma_sound_init_from_file(
            &engine,
            path.c_str(),
            MA_SOUND_FLAG_STREAM,
            nullptr,
            nullptr,
            &music) != MA_SUCCESS)
    {
        std::cerr << "Failed to load music: " << path << "\n";
        return;
    }

    ma_sound_set_looping(&music, loop ? MA_TRUE : MA_FALSE);
    ma_sound_start(&music);
    musicLoaded = true;
}

void AudioManager::stopMusic()
{
    if (!musicLoaded) return;

    ma_sound_stop(&music);
    ma_sound_uninit(&music);
    musicLoaded = false;
}

void AudioManager::shutdown()
{
    stopMusic();
    ma_engine_uninit(&engine);
}


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
    initialized = true;
    return true;
}

void AudioManager::loadSFX(const std::string& name, const std::string& path)
{
    if (sfx.contains(name))
        return;

    // Construct sound IN-PLACE
    ma_sound& sound = sfx[name];

    if (ma_sound_init_from_file(
            &engine,
            path.c_str(),
            0,          // not streamed
            nullptr,
            nullptr,
            &sound) != MA_SUCCESS)
    {
        std::cerr << "Failed to load SFX: " << path << "\n";
        sfx.erase(name);
        return;
    }
}

void AudioManager::playSFX(const std::string& name, float volume)
{
    if (!initialized)
        return;

    auto it = sfx.find(name);
    if (it == sfx.end())
        return;

    ma_sound* sound = &it->second;

    ma_sound_set_volume(sound, volume);
    ma_sound_seek_to_pcm_frame(sound, 0); // rewind
    ma_sound_start(sound);
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
    if (!initialized)
        return;

    initialized = false;

    // Stop audio thread FIRST
    ma_engine_stop(&engine);

    // Stop & uninit music
    if (musicLoaded)
    {
        ma_sound_stop(&music);
        ma_sound_uninit(&music);
        musicLoaded = false;
    }

    // Uninit SFX
    for (auto& [name, sound] : sfx)
    {
        ma_sound_stop(&sound);
        ma_sound_uninit(&sound);
    }
    sfx.clear();

    ma_engine_uninit(&engine);
}

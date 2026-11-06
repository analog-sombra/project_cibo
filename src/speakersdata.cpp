#include "speakersdata.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

std::vector<AudioDevice> SpeakerData::getAllAudioDevices()
{
    int device_count = 0;
    SDL_AudioDeviceID *devices = SDL_GetAudioPlaybackDevices(&device_count);
    std::vector<AudioDevice> audioDevices;

    if (devices)
    {
        for (int i = 0; i < device_count; ++i)
        {
            AudioDevice device;
            device.id = devices[i];
            const char *name = SDL_GetAudioDeviceName(devices[i]);
            device.name = name ? name : "Unknown Device";
            audioDevices.push_back(device);
        }
        SDL_free(devices);
        return audioDevices;
    }
    else
    {
        AudioDevice defalutdevice = getDefaultAudioDevice();
        audioDevices.push_back(defalutdevice);
        return audioDevices;
    }
}

AudioDevice SpeakerData::getDefaultAudioDevice()
{
    AudioDevice device;
    device.id = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;

    const char *name = SDL_GetAudioDeviceName(device.id);
    device.name = name ? name : "Default Device";
    return device;
}

void SpeakerData::TestAudioDevice(AudioDevice device)
{

    ma_engine engine;
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS)
    {
        SDL_Log("Failed to initialize miniaudio engine: %d", result);
        std::cout << "Failed to initialize miniaudio engine" << std::endl;
        return;
    }


    result = ma_engine_play_sound(&engine, "assets/test.mp3", NULL);
    if (result != MA_SUCCESS)
    {
        SDL_Log("Failed to play audio file: %d", result);
        std::cout << "Failed to play audio file: " << result << std::endl;
        ma_engine_uninit(&engine);
        return;
    }

    SDL_Log("Playing test audio on device: %s", device.name.c_str());

    SDL_Delay(2000);

    ma_engine_uninit(&engine);
}
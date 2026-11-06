#ifndef __SPEAKERSDATA_HPP__
#define __SPEAKERSDATA_HPP__

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <string>
#include <vector>
#include <iostream>

struct AudioDevice
{
    SDL_AudioDeviceID id;
    std::string name;
};

class SpeakerData
{

public:
    std::vector<AudioDevice> getAllAudioDevices();
    AudioDevice getDefaultAudioDevice();
    void TestAudioDevice(AudioDevice device);
};

#endif // __SPEAKERSDATA_HPP__
#pragma once

#include <vector>
#include <string>
#include <SDL3/SDL.h>

struct AudioBuffer {
    std::vector<uint8_t> pcm_data;
    int sample_rate = 24000;
    int channels = 1;
    float duration = 0.0f;
    
    bool IsValid() const {
        return !pcm_data.empty() && duration > 0.0f;
    }
    
    void Clear() {
        pcm_data.clear();
        duration = 0.0f;
    }
};

// TTS Generation
AudioBuffer GenerateSpeechWithKittenTTS(
    const std::string& text,
    const std::string& voice = "female_0",
    const std::string& language = "en",
    float speed = 1.0f,
    const std::string& temp_output = "temp_tts_output.wav"
);

std::vector<std::string> GetAvailableVoices();

// Audio Playback
bool PlayWAVFileWithSDL3(
    const std::string& filename,
    SDL_AudioDeviceID device_id
);

class AudioPlaybackManager {
private:
    SDL_AudioStream* stream = nullptr;
    bool is_playing = false;
    
public:
    bool InitializeStream(SDL_AudioDeviceID device_id, int sample_rate, 
                         int channels, SDL_AudioFormat format);
    
    bool QueueAudio(const std::vector<uint8_t>& pcm_data);
    
    bool IsPlaying() const;
    
    void Shutdown();
    
    ~AudioPlaybackManager();
};

// Complete Pipeline
bool SpeakText(
    const std::string& text,
    const std::string& voice,
    SDL_AudioDeviceID device_id
);

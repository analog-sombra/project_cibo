#include "tts_integration.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>

// ============================================================================
// TTS Generation Implementation
// ============================================================================

AudioBuffer GenerateSpeechWithKittenTTS(
    const std::string& text,
    const std::string& voice,
    const std::string& language,
    float speed,
    const std::string& temp_output
) {
    AudioBuffer result;
    
    try {
        // Build Python command
        std::string escaped_text = text;
        
        // Simple escaping (Windows)
        size_t pos = 0;
        while ((pos = escaped_text.find("\"", pos)) != std::string::npos) {
            escaped_text.replace(pos, 1, "\\\"");
            pos += 2;
        }
        
        // Build command: python tts_engine.py synthesize "text" voice output.wav
        std::string command = std::string("python tts_engine.py synthesize ") +
                            "\"" + escaped_text + "\" " +
                            voice + " " +
                            temp_output + " " +
                            language + " " +
                            std::to_string(speed);
        
        std::cerr << "[TTS] Executing: " << command << std::endl;
        
        // Execute Python TTS engine
        int ret = system(command.c_str());
        if (ret != 0) {
            std::cerr << "[TTS] Synthesis failed with code: " << ret << std::endl;
            return result;
        }
        
        // Load generated WAV file
        std::ifstream file(temp_output, std::ios::binary);
        if (!file) {
            std::cerr << "[TTS] Failed to open generated audio file: " << temp_output << std::endl;
            return result;
        }
        
        // Read file into buffer
        file.seekg(0, std::ios::end);
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (file_size <= 44) {  // WAV header is 44 bytes
            std::cerr << "[TTS] Generated file too small: " << file_size << " bytes" << std::endl;
            return result;
        }
        
        result.pcm_data.resize(file_size);
        if (!file.read(reinterpret_cast<char*>(result.pcm_data.data()), file_size)) {
            std::cerr << "[TTS] Failed to read audio file" << std::endl;
            return result;
        }
        file.close();
        
        // Calculate duration
        // WAV format: 44-byte header, then PCM data
        // KittenTTS uses 16-bit PCM, 24000 Hz, mono
        int pcm_bytes = file_size - 44;
        int sample_count = pcm_bytes / 2;  // 16-bit = 2 bytes per sample
        result.duration = sample_count / 24000.0f;
        result.sample_rate = 24000;
        result.channels = 1;
        
        std::cerr << "[TTS] Generated audio: " << result.duration << "s, "
                  << file_size << " bytes" << std::endl;
        
        // Clean up temporary file
        std::remove(temp_output.c_str());
        
    } catch (const std::exception& e) {
        std::cerr << "[TTS] Exception: " << e.what() << std::endl;
        result.Clear();
    }
    
    return result;
}

std::vector<std::string> GetAvailableVoices() {
    std::vector<std::string> voices;
    
    try {
        // Call Python to list voices (Windows uses _popen)
#ifdef _WIN32
        FILE* pipe = _popen("python tts_engine.py list_voices", "r");
#else
        FILE* pipe = popen("python tts_engine.py list_voices", "r");
#endif
        if (!pipe) {
            std::cerr << "[TTS] Failed to execute list_voices" << std::endl;
            return voices;
        }
        
        // Read output (JSON format)
        char buffer[4096];
        std::string output;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            output += buffer;
        }
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        
        // Parse JSON (simplified - just extract voice IDs)
        if (output.find("female_0") != std::string::npos) voices.push_back("female_0");
        if (output.find("female_1") != std::string::npos) voices.push_back("female_1");
        if (output.find("female_2") != std::string::npos) voices.push_back("female_2");
        if (output.find("male_0") != std::string::npos) voices.push_back("male_0");
        if (output.find("male_1") != std::string::npos) voices.push_back("male_1");
        if (output.find("male_2") != std::string::npos) voices.push_back("male_2");
        
    } catch (const std::exception& e) {
        std::cerr << "[TTS] Error getting voices: " << e.what() << std::endl;
    }
    
    return voices;
}

// ============================================================================
// Audio Playback Implementation
// ============================================================================

bool PlayWAVFileWithSDL3(
    const std::string& filename,
    SDL_AudioDeviceID device_id
) {
    // Open file using SDL3's IO system
    SDL_IOStream* io = SDL_IOFromFile(filename.c_str(), "rb");
    if (!io) {
        std::cerr << "[AUDIO] Failed to open WAV file: " << filename << std::endl;
        return false;
    }
    
    // Load WAV
    SDL_AudioSpec spec;
    uint8_t* audio_buf = nullptr;
    uint32_t audio_len = 0;
    
    if (!SDL_LoadWAV_IO(io, 1, &spec, &audio_buf, &audio_len)) {
        std::cerr << "[AUDIO] Failed to load WAV: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cerr << "[AUDIO] Loaded WAV: " << audio_len << " bytes, "
              << spec.freq << "Hz, " << (int)spec.channels << " channels" << std::endl;
    
    // Create audio stream to target device with proper spec
    SDL_AudioStream* stream = SDL_CreateAudioStream(&spec, &spec);
    if (!stream) {
        std::cerr << "[AUDIO] Failed to create audio stream" << std::endl;
        SDL_free(audio_buf);
        return false;
    }
    
    // Bind stream to device
    if (!SDL_BindAudioStream(device_id, stream)) {
        std::cerr << "[AUDIO] Failed to bind audio stream to device" << std::endl;
        SDL_DestroyAudioStream(stream);
        SDL_free(audio_buf);
        return false;
    }
    
    // Queue audio data
    if (SDL_PutAudioStreamData(stream, audio_buf, audio_len) != 0) {
        std::cerr << "[AUDIO] Failed to queue audio data" << std::endl;
        SDL_DestroyAudioStream(stream);
        SDL_free(audio_buf);
        return false;
    }
    
    // Start playback (stream is already resumed when bound to device)
    std::cerr << "[AUDIO] Playback started" << std::endl;
    
    // Wait for playback to complete
    while (SDL_GetAudioStreamQueued(stream) > 0) {
        SDL_Delay(10);
    }
    
    std::cerr << "[AUDIO] Playback completed" << std::endl;
    
    // Cleanup
    SDL_DestroyAudioStream(stream);
    SDL_free(audio_buf);
    
    return true;
}

// ============================================================================
// AudioPlaybackManager Implementation
// ============================================================================

bool AudioPlaybackManager::InitializeStream(SDL_AudioDeviceID device_id, int sample_rate, 
                                           int channels, SDL_AudioFormat format) {
    if (stream) {
        SDL_DestroyAudioStream(stream);
    }
    
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = sample_rate;
    desired.channels = channels;
    desired.format = format;
    
    // Create stream with proper format specs
    stream = SDL_CreateAudioStream(&desired, &desired);
    if (!stream) {
        std::cerr << "[AUDIO] Failed to create stream" << std::endl;
        return false;
    }
    
    // Bind to device
    if (!SDL_BindAudioStream(device_id, stream)) {
        std::cerr << "[AUDIO] Failed to bind stream to device" << std::endl;
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
        return false;
    }
    
    // Resume the stream for playback
    if (!SDL_ResumeAudioDevice(device_id)) {
        std::cerr << "[AUDIO] Failed to resume playback" << std::endl;
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
        return false;
    }
    
    return true;
}

bool AudioPlaybackManager::QueueAudio(const std::vector<uint8_t>& pcm_data) {
    if (!stream) {
        std::cerr << "[AUDIO] Stream not initialized" << std::endl;
        return false;
    }
    
    if (SDL_PutAudioStreamData(stream, pcm_data.data(), pcm_data.size()) != 0) {
        std::cerr << "[AUDIO] Failed to queue audio" << std::endl;
        return false;
    }
    
    is_playing = true;
    return true;
}

bool AudioPlaybackManager::IsPlaying() const {
    if (!stream) return false;
    
    int queued = SDL_GetAudioStreamQueued(stream);
    if (queued == 0 && is_playing) {
        const_cast<AudioPlaybackManager*>(this)->is_playing = false;
    }
    return is_playing;
}

void AudioPlaybackManager::Shutdown() {
    if (stream) {
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
    }
    is_playing = false;
}

AudioPlaybackManager::~AudioPlaybackManager() {
    Shutdown();
}

// ============================================================================
// Complete TTS + Audio Pipeline
// ============================================================================

bool SpeakText(
    const std::string& text,
    const std::string& voice,
    SDL_AudioDeviceID device_id
) {
    std::cerr << "[SPEAK] Generating audio..." << std::endl;
    AudioBuffer audio = GenerateSpeechWithKittenTTS(text, voice);
    
    if (!audio.IsValid()) {
        std::cerr << "[SPEAK] Failed to generate audio" << std::endl;
        return false;
    }
    
    std::cerr << "[SPEAK] Playing audio to device " << device_id << std::endl;
    bool success = PlayWAVFileWithSDL3("temp_tts_output.wav", device_id);
    
    return success;
}

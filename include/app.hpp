#ifndef __APP_HPP__
#define __APP_HPP__

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include "tts_integration.hpp"
#include "speakersdata.hpp"
#include <SDL3_mixer/SDL_mixer.h>

class Application
{
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool running = true;
    bool showAboutPanel = false;
    bool showSpeakerPanel = true;

    // Speaker management
    SpeakerData speakerData;
    std::vector<AudioDevice> audioDevices;
    int selectedSpeakerIndex = 0;
    bool isTesting = false;

    // Private initialization methods
    void initializeSDL();
    void initializeRenderer();
    void initializeAudio();
    void initializeImGui();
    void setupImGuiStyle();
    void cleanupImGui();
    void cleanupRenderer();

    // Private rendering methods
    void renderAboutPanel(ImGuiID dockspace_id);
    void renderSpeakerPanel(ImGuiID dockspace_id);
    void presentFrame();

public:
    Application();
    ~Application();

    void run();
    void handleEvents();
    void render();
};
#endif // __APP_HPP__
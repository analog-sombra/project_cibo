#include "app.hpp"

namespace
{
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr int BG_COLOR_R = 45;
    constexpr int BG_COLOR_G = 45;
    constexpr int BG_COLOR_B = 48;
    constexpr int FRAME_DELAY_MS = 16;
}

Application::Application()
{
    showAboutPanel = true;
    showSpeakerPanel = true;
    initializeSDL();
    initializeRenderer();
    initializeAudio();
    initializeImGui();
    audioDevices = speakerData.getAllAudioDevices();
}

Application::~Application()
{
    cleanupImGui();
    cleanupRenderer();
    SDL_Quit();
}

void Application::initializeSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    }
}

void Application::initializeRenderer()
{
    window = SDL_CreateWindow(
        "TTS Application with ImGui Docking",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

void Application::initializeAudio()
{
    if (!MIX_Init())
    {
        SDL_Log("Failed to initialize SDL_mixer: %s", SDL_GetError());
    }
}

void Application::initializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    setupImGuiStyle();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void Application::setupImGuiStyle()
{
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    ImGui::StyleColorsDark();
}

void Application::cleanupImGui()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void Application::cleanupRenderer()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
        SDL_DestroyWindow(window);
    }
}

void Application::run()
{
    while (running)
    {
        handleEvents();
        render();
    }
}

void Application::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
        }
    }
}

void Application::render()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    renderAboutPanel(dockspace_id);
    renderSpeakerPanel(dockspace_id);

    ImGui::Render();
    presentFrame();
}

void Application::renderAboutPanel(ImGuiID dockspace_id)
{
    if (!showAboutPanel)
    {
        return;
    }

    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("About TTS", &showAboutPanel))
    {
        ImGui::Text("Text-to-Speech Application");
        ImGui::Text("Using ImGui with SDL3 Audio Device Detection");
        ImGui::Spacing();

        ImGui::Text("Features:");
        ImGui::BulletText("Multi-line text input");
        ImGui::BulletText("System audio device detection (speakers, headphones)");
        ImGui::BulletText("4 different voice profiles");
        ImGui::BulletText("Dockable window layout");

        ImGui::Separator();
        ImGui::Text("How to use:");
        ImGui::BulletText("1. Select audio output device (speakers/headphones)");
        ImGui::BulletText("2. Choose voice profile");
        ImGui::BulletText("3. Enter text to speak");
        ImGui::BulletText("4. Click 'Speak' to playback");
        ImGui::BulletText("5. Drag window headers to customize layout");
        ImGui::Separator();
    }
    ImGui::End();
}

void Application::renderSpeakerPanel(ImGuiID dockspace_id)
{
    if (!showSpeakerPanel)
    {
        return;
    }

    static bool first_time = true;
    if (first_time)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        first_time = false;
    }

    if (ImGui::Begin("Speaker Selection", &showSpeakerPanel))
    {
        ImGui::Text("Speaker Control Panel");
        ImGui::Text("Total devices: %d", (int)audioDevices.size());
        
        if (audioDevices.empty())
        {
            ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "No audio devices found!");
        }
        else
        {
            ImGui::Separator();
            ImGui::Text("Available Speakers:");
            
            if (ImGui::BeginChild("##speaker_list", ImVec2(0, 150), true))
            {
                for (int i = 0; i < (int)audioDevices.size(); i++)
                {
                    if (ImGui::Selectable(audioDevices[i].name.c_str(), selectedSpeakerIndex == i))
                    {
                        selectedSpeakerIndex = i;
                    }
                }
                ImGui::EndChild();
            }
            
            ImGui::Separator();
            if (selectedSpeakerIndex < (int)audioDevices.size())
            {
                ImGui::Text("Current: %s", audioDevices[selectedSpeakerIndex].name.c_str());
                
                if (ImGui::Button("Test Speaker", ImVec2(-1, 0)))
                {
                    speakerData.TestAudioDevice(audioDevices[selectedSpeakerIndex]);
                }
            }
        }
    }
    ImGui::End();
}

void Application::presentFrame()
{
    SDL_SetRenderDrawColor(renderer, BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(FRAME_DELAY_MS);
}

#include <iostream>
#include <filesystem>

#include "Helper.h"
#include "SDL2/SDL.h"
#include "SDL_image/SDL_image.h"
#include "Engine.h"
#include "ActorTemplateDB.h"
#include "AudioDB.h"
#include "TextDB.h"
#include "Input.h"
#include "ComponentDB.h"
#include "Rigidbody.h"
#include "Eventbus.h"
#include "ParticleSystem.h"
#include "ThreadPool.h"
#include "Timer.h"

Engine::Engine(){
    {
        SCOPED_TIMER("init");
        if(!std::filesystem::exists("resources")){
            std::cout << "error: resources/ missing";
            exit(0);
        }

        ThreadPool::Init();
        Renderer::init();
        TextDB::init();
        AudioDB::init();
        ComponentDB::Init();
        ImageDB::PreloadAll();
        ActorTemplateDB::init();
        SceneDB::init();
        ParticleSystem::Init();
    }
    
    render_thread = std::thread(&Engine::RenderLoop, this);
}

void Engine::GameLoop(){   
    SCOPED_TIMER("game");

    SceneDB::runAllOnStarts();
    while(isRunning.load()){
        WaitForPreviousFrameDone();
        Input();
        frame_ready.notify_one();
        Update();
    }

    frame_ready.notify_one();
    render_thread.join();
    ThreadPool::Shutdown(); 
};

void Engine::Input(){
    while(Helper::SDL_PollEvent(&last_event)){
        if(last_event.type == SDL_QUIT){
            isRunning.store(false);
        }
        Input::ProcessEvent(last_event);
    }
}

void Engine::Update(){
    SceneDB::ifStagedSceneThenLoad();

    SceneDB::runAllOnStarts();
    SceneDB::runAllUpdates();

    // Late Updates
    SceneDB::runAllLateUpdates();
    SceneDB::addRemoveComponentsRunTime();
    SceneDB::addRemoveActorsRunTime();
    SceneDB::runAllOnDestroys();
    Eventbus::AddRemovePending();
    Input::LateUpdate();
    Rigidbody::Advance();

}

void Engine::RenderLoop(){
    int read_idx;
    while(WaitForNextFrame(read_idx)){
        Renderer::rendererClear();
        Renderer::renderAllImages(read_idx);
        Renderer::renderAllUI(read_idx);
        Renderer::renderAllTexts(read_idx);
        Renderer::renderAllPixels(read_idx);

        Helper::SDL_RenderPresent(Renderer::GetRenderer());
        MarkFrameConsumed();
    }
}

// --- helpers ---

void Engine::WaitForPreviousFrameDone() {
    std::unique_lock<std::mutex> lock(frame_mutex);
    frame_consumed.wait(lock, [&]{ return !frame_pending; });
    frame_pending = true;
    Renderer::write_index.fetch_xor(1);
}

bool Engine::WaitForNextFrame(int& read_idx) {
    std::unique_lock<std::mutex> lock(frame_mutex);
    frame_ready.wait(lock, [&]{ return frame_pending || !isRunning.load(); });
    if (!isRunning.load()) return false;
    read_idx = 1 - Renderer::write_index.load();
    return true;
}

void Engine::MarkFrameConsumed() {
    {
        std::lock_guard<std::mutex> lock(frame_mutex);
        frame_pending = false;
    }
    frame_consumed.notify_one();
}
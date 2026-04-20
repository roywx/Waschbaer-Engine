#ifndef ENGINE_H
#define ENGINE_H

#include "SceneDB.h"
#include "Renderer.h"
#include "SDL2/SDL.h"
#include "ImageDB.h"

#include <thread>
#include <mutex>
#include <condition_variable>

class Engine{
public:
    std::string last_input;
    SDL_Event last_event;

    void GameLoop();
    Engine();
    
private:
    std::thread render_thread;
    std::mutex frame_mutex;
    std::condition_variable frame_ready;
    std::condition_variable frame_consumed;
    std::atomic<bool> isRunning{true};
    bool frame_pending = false;

    void Input();
    void Update();
   
    void RenderLoop();
    void ParticleLoop();

    void WaitForPreviousFrameDone(); 
    bool WaitForNextFrame(int& read_idx);  
    void MarkFrameConsumed();
};



#endif
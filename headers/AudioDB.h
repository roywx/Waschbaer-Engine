#ifndef AUDIO_DB_H
#define AUDIO_DB_H

#include <string>
#include <unordered_map> 

#include "rapidjson/document.h"
#include "SDL2/SDL.h"
#include "SDL_ttf/SDL_ttf.h"
#include "SDL_mixer/SDL_mixer.h"

class AudioDB{
private:
    static inline std::unordered_map<std::string, Mix_Chunk*> loaded_audio;
    static void LoadAudio(const std::string& audio_name);
public:
    static void init();
    static bool isAudioLoaded(const std::string& audio_name);
    static void PlayAudio(int channel, const std::string& audio_name, bool loop);
    static void stopAudioChannel(int channel);
    static void setVolume(int channel, int volume);
};

#endif
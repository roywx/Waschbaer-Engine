#include "AudioDB.h"
#include "EngineUtils.h"
#include "AudioHelper.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <functional> 


void AudioDB::init(){
    AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    AudioHelper::Mix_AllocateChannels(50);
}

void AudioDB::LoadAudio(const std::string& audio_name){
    std::string RESOURCE_PATH = "resources/audio/";
  
    std::string filepath = RESOURCE_PATH + audio_name;  

    if(std::filesystem::exists(filepath + ".wav")){
        filepath += ".wav";
    }else if(std::filesystem::exists(filepath + ".ogg")){
        filepath += ".ogg";
    }else{
        std::cout <<  "error: failed to play audio clip " + audio_name;
        exit(0);
    }
    
    loaded_audio[audio_name] = AudioHelper::Mix_LoadWAV(filepath.c_str());
}

bool AudioDB::isAudioLoaded(const std::string& audio_name){
    return loaded_audio.count(audio_name) && loaded_audio[audio_name];
}

void AudioDB::PlayAudio(int channel, const std::string& audio_name, bool loop){
    if(!loaded_audio.count(audio_name)){
        LoadAudio(audio_name);
    }
    int num = (loop) ? -1 : 0;
    AudioHelper::Mix_PlayChannel(channel, loaded_audio[audio_name], num);
}

void AudioDB::stopAudioChannel(int channel){
    AudioHelper::Mix_HaltChannel(channel);
}

void AudioDB::setVolume(int channel, int volume){
    AudioHelper::Mix_Volume(channel, volume);
}
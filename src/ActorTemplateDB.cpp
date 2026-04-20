#include "ActorTemplateDB.h"
#include "EngineUtils.h"
#include "Renderer.h"
#include "Helper.h"
#include "ThreadPool.h"

#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <string>

ActorData* ActorTemplateDB::getTemplate(std::string name){
    if(!actorTemplates.count(name)){
        std::cout << "error: template " << name << " is missing";
        exit(0);
    }
    return &ActorTemplateDB::actorTemplates[name];
}

void ActorTemplateDB::init(){
    std::string dir_path = "resources/actor_templates";
    if(!std::filesystem::exists(dir_path)) return;
    
    std::vector<std::filesystem::path> paths;
    for(const auto& entry : std::filesystem::directory_iterator(dir_path)){
        paths.push_back(entry.path());
    }

    struct ParsedTemplate {
        std::string name;
        rapidjson::Document doc;
    };
    
    std::vector<ParsedTemplate> parsed(paths.size());

    ThreadPool::ParallelFor((int)paths.size(), [&](int start, int end){
        for(int i = start; i < end; i++){
            parsed[i].name = paths[i].stem().string();
            EngineUtils::ReadJsonFile(paths[i].string(), parsed[i].doc);
        }
    });

    for(auto& p : parsed){
        actorTemplates[p.name] = ActorData(p.doc);
    }
}
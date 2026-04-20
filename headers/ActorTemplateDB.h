#ifndef ACTOR_TEMPLATE_DB_H
#define ACTOR_TEMPLATE_DB_H

#include <unordered_map>
#include <string>
#include <memory>

#include "Actor.h"

class ActorTemplateDB{
private:
    static inline std::unordered_map<std::string, ActorData> actorTemplates;
public:
    static void init();
    static ActorData* getTemplate(std::string name);    
    
};
#endif
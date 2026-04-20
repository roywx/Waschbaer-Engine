#include "Eventbus.h"

void Eventbus::AddRemovePending(){
    for(luaref_trip trip : pending_subscribers){
        events[std::get<0>(trip)].push_back(luaref_pair(std::get<1>(trip), std::get<2>(trip)));
    }
    pending_subscribers.clear();

    for(luaref_trip trip : pending_unsubscribers){
        for(auto it = events[std::get<0>(trip)].begin(); it != events[std::get<0>(trip)].end(); ){
            if(std::get<0>(*it).rawequal(std::get<1>(trip)) &&   // component == component
            std::get<1>(*it).rawequal(std::get<2>(trip))){     // function == function
                it = events[std::get<0>(trip)].erase(it);
            } else {
                ++it;
            }
        }
    }
    pending_unsubscribers.clear();
}


void Eventbus::Publish(std::string event_type, luabridge::LuaRef event_object){
    for(luaref_pair& pair : events[event_type]){
        std::get<1>(pair)(std::get<0>(pair), event_object);
    }
}

void Eventbus::Subscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function){
    pending_subscribers.push_back(luaref_trip(event_type, component, function));
}

void Eventbus::Unsubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function){
    pending_unsubscribers.push_back(luaref_trip(event_type, component, function));
}


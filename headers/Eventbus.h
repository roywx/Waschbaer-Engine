#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <string>
#include <tuple>    
#include <unordered_map>

class Eventbus{

using luaref_pair = std::tuple<luabridge::LuaRef, luabridge::LuaRef>;
using luaref_trip = std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>;

private:
    static inline std::unordered_map<std::string, std::vector<luaref_pair>> events;
    static inline std::vector<luaref_trip> pending_subscribers;
    static inline std::vector<luaref_trip> pending_unsubscribers;

public:
    static void Publish(std::string event_type, luabridge::LuaRef event_object);
    static void Subscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void Unsubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void AddRemovePending();
};

#endif
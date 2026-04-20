#ifndef COMPONENT_DB_H
#define COMPONENT_DB_H
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <unordered_map>
#include <string>

class ComponentDB{
public:
    static void Init(); 
    static void InitState();
    static void InitFunctions();
    static void InitComponents();
    static void EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table);
    static void CreateNewInstance(luabridge::LuaRef& parent_table);
    static luabridge::LuaRef CreateComponent(const std::string& type_name, const std::string& component_key);
    static luabridge::LuaRef GetNil();

    static inline lua_State* lua_state;
    static inline std::unordered_map<std::string, luabridge::LuaRef> component_table;
private:
    static inline std::string COMPONENT_FILEPATH = "resources/component_types";
    static void CppLog(const std::string& message);
};

#endif
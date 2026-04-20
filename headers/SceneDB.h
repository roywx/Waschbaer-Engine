#ifndef SCENE_DB_H
#define SCENE_DB_H

#include <filesystem>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

#include "SlotMap.h"
#include "glm/glm.hpp"
#include "EngineUtils.h"
#include "rapidjson/document.h"
#include "Actor.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

class SceneDB{
private:
    static inline std::string curr_scene_name;
    static inline bool scene_staged = false;
    static inline std::string staged_scene_name;

    static inline std::vector<std::unique_ptr<Actor>> to_add_actors;
    static inline std::vector<Actor*> to_remove_actors;
        
    static inline std::vector<luabridge::LuaRef> on_starts_refs;
    static inline std::vector<luabridge::LuaRef> on_update_refs;
    static inline std::vector<luabridge::LuaRef> on_late_update_refs;
    static inline std::vector<luabridge::LuaRef> on_destroy_refs;
    
    static bool loadScene(const std::string& scene_name);
public:
    static inline std::vector<std::unique_ptr<Actor>> actors;  

    static inline std::vector<luabridge::LuaRef> to_delete_refs;

    static void init();
    static void stageNewScene(const std::string& scene_name);
    static std::string GetCurrentScene();
    
    static void ifStagedSceneThenLoad();

    static void runAllOnStarts();
    static void runAllUpdates();
    static void runAllLateUpdates();
    static void runAllOnDestroys();

    static void addRemoveComponentsRunTime();

    static luabridge::LuaRef InstantiateActor(const std::string& actor_template_name);
    static void DestroyActor(Actor* ref);
    static void addRemoveActorsRunTime();
    static void ProtectActorOnSceneChange(Actor* ref);
    
    static luabridge::LuaRef FindActor(const std::string& name);
	static luabridge::LuaRef FindAllActors(const std::string& name);
};

#endif
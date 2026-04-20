#include "SceneDB.h"
#include "rapidjson/document.h"
#include "ActorTemplateDB.h"
#include "Renderer.h"
#include "SDL2/SDL.h"
#include "SDL_image/SDL_image.h"
#include "Helper.h"
#include "AudioDB.h"
#include "Rigidbody.h"
#include "ParticleSystem.h"

#include <iostream>
#include <algorithm>
#include <cmath>

void SceneDB::init(){
    rapidjson::Document d_Config;
    EngineUtils::ReadJsonFile("resources/game.config", d_Config);

    bool success = EngineUtils::json_try_get_value(d_Config, "initial_scene", curr_scene_name);
    if(!success){
        std::cout << "error: initial_scene unspecified";
        exit(0);
    }    
    loadScene(curr_scene_name);
}

bool SceneDB::loadScene(const std::string& scene_name){
    rapidjson::Document d_level;
    std::string levelFilePath = "resources/scenes/" + scene_name + ".scene";
    
    if(std::filesystem::exists(levelFilePath)){
        EngineUtils::ReadJsonFile(levelFilePath, d_level);
    }else{
        std::cout << "error: scene " + scene_name + " is missing";
        exit(0);
    }
    
    on_starts_refs.clear();
    on_update_refs.clear();
    on_late_update_refs.clear();
    to_delete_refs.clear();
    
    for(auto iterator = actors.begin(); iterator != actors.end();){
        if(iterator->get()->scene_swap_protected){ ++iterator; continue;}
        if(!iterator->get()->to_delete){
            for(auto& [key, component] : iterator->get()->data.actor_components){
                if(component.lua_ref["OnDestroy"].isFunction()){
                    on_destroy_refs.push_back(component.lua_ref);
                }
            }
        }
        
        actors.erase(iterator);
    }
    for(auto iterator = to_add_actors.begin(); iterator != to_add_actors.end();){
        if(iterator->get()->scene_swap_protected) { ++iterator; continue;}
        if(!iterator->get()->to_delete){
            for(auto& [key, component] : iterator->get()->data.actor_components){
                if(component.lua_ref["OnDestroy"].isFunction()){
                    on_destroy_refs.push_back(component.lua_ref);
                }
            }
        }
        to_add_actors.erase(iterator);
    }

    // Add back functions removed
    for(auto& actor : actors){
        for(auto& pair : actor->data.actor_components){
            auto& ref = pair.second.lua_ref;
            if(ref["OnUpdate"].isFunction())
                on_update_refs.push_back(ref);
            if(ref["OnLateUpdate"].isFunction())
                on_late_update_refs.push_back(ref);
        }
    }
    for(auto& actor : to_add_actors){
        for(auto& pair : actor->data.actor_components){
            auto& ref = pair.second.lua_ref;
            if(ref["OnUpdate"].isFunction())
                on_update_refs.push_back(ref);
            if(ref["OnLateUpdate"].isFunction())
                on_late_update_refs.push_back(ref);
        }
    }

    to_remove_actors.clear();
    
    scene_staged = false;
    curr_scene_name = scene_name;
    staged_scene_name.clear();


    if(d_level.HasMember("actors") && d_level["actors"].IsArray()) {
        const auto& actorsArray = d_level["actors"].GetArray();
        actors.reserve(actorsArray.Size());

        for(rapidjson::Value& obj : actorsArray) {
            if(!obj.IsObject()) continue;
            std::unique_ptr<Actor> newActor = std::make_unique<Actor>();
            if(obj.HasMember("template")){
                // Copy Actor Template Data
                newActor->data = *ActorTemplateDB::getTemplate(obj["template"].GetString());
               
                /* Make a new component instance for each component instance inherited from template;
                If the inherited componenet is overwritten then original component *should* be garbaged collected by Lua;
                TODO: Creating lua objects for ActorTemplates seems to be unnecessary, could possibly just store the type to 
                save on memory?
                TODO: More generalizable way to do C++ components
                */

                for(auto& [key, component] : newActor->data.actor_components){
                    if(component.type == "Rigidbody"){
                        Rigidbody* parent_rb = component.lua_ref.cast<Rigidbody*>();
                        Rigidbody* new_rb = new Rigidbody(*parent_rb);
                        component.lua_ref = luabridge::LuaRef(ComponentDB::lua_state, new_rb);
                    }else if(component.type == "ParticleSystem"){
                        ParticleSystem* parent_rb = component.lua_ref.cast<ParticleSystem*>();
                        ParticleSystem* new_rb = new ParticleSystem(*parent_rb);
                        component.lua_ref = luabridge::LuaRef(ComponentDB::lua_state, new_rb);
                    }else{
                        luabridge::LuaRef new_component = luabridge::newTable(ComponentDB::lua_state);
                        ComponentDB::EstablishInheritance(new_component, component.lua_ref);
                        component.lua_ref = new_component;
                    }
                }
                // Overwrite data
                newActor->data.initActorData(obj);
            }else{
                newActor->data.initActorData(obj);
            }
            for(auto& pair : newActor->data.actor_components){
                pair.second.lua_ref["actor"] = newActor.get();
                if(pair.second.lua_ref["OnStart"].isFunction()){
                    on_starts_refs.push_back(pair.second.lua_ref);
                }
                if(pair.second.lua_ref["OnUpdate"].isFunction()){
                    on_update_refs.push_back(pair.second.lua_ref);
                }
                if(pair.second.lua_ref["OnLateUpdate"].isFunction()){
                    on_late_update_refs.push_back(pair.second.lua_ref);
                }
            }
            actors.push_back(std::move(newActor));

        }
    }
    return true;
}

std::string SceneDB::GetCurrentScene(){
    return curr_scene_name;
}

void SceneDB::ProtectActorOnSceneChange(Actor* ref){
    ref->scene_swap_protected = true;
}

void SceneDB::stageNewScene(const std::string& scene_name){
    scene_staged = true;
    staged_scene_name = scene_name;
}

void SceneDB::ifStagedSceneThenLoad(){
    if(scene_staged){
        loadScene(staged_scene_name);
    }
}

template<bool CheckEnabled = true>
inline void runLuaRefs(std::vector<luabridge::LuaRef>& refs, const std::string& funcName) {
    for (luabridge::LuaRef& luaref : refs) {
        try {
            if constexpr (CheckEnabled) {
                if (!luaref["enabled"]) continue;
            }
            luaref[funcName](luaref);
        } catch (const luabridge::LuaException& e) {
            EngineUtils::ReportError(luaref["actor"].cast<Actor>().GetName(), e);
        }
    }
}

void SceneDB::runAllOnStarts()    { runLuaRefs(on_starts_refs,     "OnStart");    on_starts_refs.clear(); }
void SceneDB::runAllUpdates()     { runLuaRefs(on_update_refs,      "OnUpdate"); }
void SceneDB::runAllLateUpdates() { runLuaRefs(on_late_update_refs, "OnLateUpdate"); }
void SceneDB::runAllOnDestroys()  { runLuaRefs<false>(on_destroy_refs, "OnDestroy"); on_destroy_refs.clear(); }

void SceneDB::addRemoveComponentsRunTime(){
    for(auto& a : SceneDB::actors){
        for(auto& [key, new_component] : a->runtime_components_to_add){
            if(new_component.lua_ref["OnStart"].isFunction()) on_starts_refs.push_back(new_component.lua_ref);
            if(new_component.lua_ref["OnUpdate"].isFunction()) on_update_refs.push_back(new_component.lua_ref);
            if(new_component.lua_ref["OnLateUpdate"].isFunction()) on_late_update_refs.push_back(new_component.lua_ref);
        }
        a->data.actor_components.merge(a->runtime_components_to_add);
        assert(a->runtime_components_to_add.empty()); // Should be empty since there should be no conflicting keys

        for(std::string& key : a->components_to_remove){
            a->data.actor_components.erase(key);
        }
        a->components_to_remove.clear();
    }

    for(auto& a : SceneDB::to_add_actors){
        // Don't need to check for OnStart/OnUpdate/OnLateUpdate, addRemoveActorRunTime() will do this for us        
        a->data.actor_components.merge(a->runtime_components_to_add);
        assert(a->runtime_components_to_add.empty()); // Should be empty since there should be no conflicting keys

        for(std::string& key : a->components_to_remove){
            a->data.actor_components.erase(key);
        }
        a->components_to_remove.clear();
    }
    //TODO: speed this up, use SlotMap? 
    for (const luabridge::LuaRef& del_ref : to_delete_refs) {
        if(del_ref["OnDestroy"].isFunction()){
            on_destroy_refs.push_back(del_ref);
        }
        on_update_refs.erase(
            std::remove_if(on_update_refs.begin(), on_update_refs.end(),
                [&](const luabridge::LuaRef& ref){
                    return del_ref.rawequal(ref);
                }),
            on_update_refs.end()
        );

        on_late_update_refs.erase(
            std::remove_if(on_late_update_refs.begin(), on_late_update_refs.end(),
                [&](const luabridge::LuaRef& ref){
                    return del_ref.rawequal(ref);
                }),
            on_late_update_refs.end()
        );
    }
    to_delete_refs.clear();
}

void SceneDB::addRemoveActorsRunTime(){
    for(auto& actor : to_add_actors){
        for(auto& component : actor->data.actor_components){
            auto& ref = component.second.lua_ref;
            if(ref["OnStart"].isFunction())
                on_starts_refs.push_back(ref);
            if(ref["OnUpdate"].isFunction())
                on_update_refs.push_back(ref);
            if(ref["OnLateUpdate"].isFunction())
                on_late_update_refs.push_back(ref);
        }
        actors.push_back(std::move(actor));
    }
    to_add_actors.clear();

    for(Actor* actor_ptr: to_remove_actors){
        for(auto it = actors.begin(); it != actors.end(); it++){
            if(it->get() == actor_ptr){
                actors.erase(it);
                break;
            }
        }
    }
    to_remove_actors.clear();
}

luabridge::LuaRef SceneDB::FindActor(const std::string& name){
    for(auto& a : actors){
        if(!a->to_delete && a->data.actor_name == name){
            return luabridge::LuaRef(ComponentDB::lua_state, a.get());
        }
    }
    for(auto& a : to_add_actors){
         if(!a->to_delete && a->data.actor_name == name){
            return luabridge::LuaRef(ComponentDB::lua_state, a.get());
        }
    }
    return ComponentDB::GetNil();
}

luabridge::LuaRef SceneDB::FindAllActors(const std::string& name){
    luabridge::LuaRef table = luabridge::newTable(ComponentDB::lua_state);
    int idx = 1;
	for(auto& a : actors){
		if(!a->to_delete && a->data.actor_name == name){
            table[idx] = a.get();
            idx++;
        }
	}
    for(auto& a : to_add_actors){
		if(!a->to_delete && a->data.actor_name == name){
            table[idx] = a.get();
            idx++;
        }
	}
	return table;
}

luabridge::LuaRef SceneDB::InstantiateActor(const std::string& actor_template_name){
    std::unique_ptr<Actor> newActor = std::make_unique<Actor>();
    newActor->data = *ActorTemplateDB::getTemplate(actor_template_name);
               
    /* Make a new component instance for each component instance inherited from template;
    TODO: Creating lua objects for ActorTemplates seems to be unnecessary, could possibly just store the type to 
    save on memory?
    TODO: More generalizable way to do C++ Components
    */ 
    for(auto& [key, component] : newActor->data.actor_components){
        if(component.type == "Rigidbody"){
            Rigidbody* parent_rb = component.lua_ref.cast<Rigidbody*>();
            Rigidbody* new_rb = new Rigidbody(*parent_rb);
            component.lua_ref = luabridge::LuaRef(ComponentDB::lua_state, new_rb);
        }else if(component.type == "ParticleSystem"){
            ParticleSystem* parent_rb = component.lua_ref.cast<ParticleSystem*>();
            ParticleSystem* new_rb = new ParticleSystem(*parent_rb);
            component.lua_ref = luabridge::LuaRef(ComponentDB::lua_state, new_rb);
        }else {
            luabridge::LuaRef new_component = luabridge::newTable(ComponentDB::lua_state);
            ComponentDB::EstablishInheritance(new_component, component.lua_ref);
            component.lua_ref = new_component;
        }
        component.lua_ref["actor"] = newActor.get();
    }

    to_add_actors.push_back(std::move(newActor));

    return luabridge::LuaRef(ComponentDB::lua_state, to_add_actors.back().get());
}

void SceneDB::DestroyActor(Actor* actor){
    for(auto& [key, component] : actor->data.actor_components){
        component.lua_ref["enabled"] = false;
     // Currently not removing components from onupdate and onlateupdates for destroyActor
     // Much faster not to, TODO: fix this
     //   to_delete_refs.push_back(component.lua_ref);
    
        if(component.lua_ref["OnDestroy"].isFunction()){
            on_destroy_refs.push_back(component.lua_ref);
        }
    }
    actor->to_delete = true;
    to_remove_actors.push_back(actor);
}
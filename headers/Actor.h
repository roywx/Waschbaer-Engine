#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <optional>
#include <map>
#include <vector>

#include "glm/glm.hpp"
#include <iostream>
#include "SDL2/SDL.h"
#include "SDL_image/SDL_image.h"
#include "rapidjson/document.h"
#include "ComponentDB.h"
#include "ContactListener.h"

// TODO: Look into getting rid of actor_component and having each actor have a map to luarefs?
struct actor_component{
	bool to_delete = false;
	std::string type; 
	luabridge::LuaRef lua_ref;

	actor_component(const std::string& t, const luabridge::LuaRef& r)
        : type(t), lua_ref(r) {}
};

struct ActorData{
	std::string actor_name = "";
	std::map<std::string, actor_component> actor_components;

	void initActorData(const rapidjson::Value& obj);

	ActorData() {};
	ActorData(const rapidjson::Value& obj);
};

class Actor{
private:
	static inline int g_uuid = 0; 
public:
	bool to_delete = false;
	bool scene_swap_protected = false;

	int actorID;
	int numRuntimeAddedComponents = 0;
	ActorData data;
	std::map<std::string, actor_component> runtime_components_to_add;
	std::vector<std::string> components_to_remove; // store by key

	Actor();

	std::string GetName();
	int GetID();

	void OnCollisionEnter(Collision c);
	void OnCollisionExit(Collision c);
	void OnTriggerEnter(Collision c);
	void OnTriggerExit(Collision c);

	luabridge::LuaRef addRunTimeComponent(const std::string& type_name);
	void removeComponent(luabridge::LuaRef component_ref);

	luabridge::LuaRef GetComponentByKey(const std::string& key);
	luabridge::LuaRef GetComponentByType(const std::string& type);
	luabridge::LuaRef GetComponentsOfType(const std::string& type);
};


#endif
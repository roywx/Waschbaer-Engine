#include "Actor.h"
#include "EngineUtils.h"
#include "ImageDB.h"
#include "Rigidbody.h"

#include <iostream>
#include <cstdlib>
#include <Renderer.h>
#include <Helper.h>

void ActorData::initActorData(const rapidjson::Value& obj){

	EngineUtils::json_try_get_value(obj, "name", actor_name);

	const auto component_it = obj.FindMember("components");
	if(component_it == obj.MemberEnd()){
		return;
	}
	const rapidjson::Value& components_obj = component_it->value;

	for(auto it = components_obj.MemberBegin(); it != components_obj.MemberEnd(); ++it){
		const std::string component_key = it->name.GetString();   
		const rapidjson::Value& component_info = it->value;      

		/* If find component type then add new instance to actor */
		const auto type_it = component_info.FindMember("type");
		if(type_it != component_info.MemberEnd()){
			const std::string type = type_it->value.GetString();
			actor_components.insert_or_assign(component_key, actor_component(type, ComponentDB::CreateComponent(type, component_key)));
		}
		assert(actor_components.count(component_key));
		
		// Look through component info for overrides
		for(auto info_it = component_info.MemberBegin(); info_it != component_info.MemberEnd(); ++info_it){
			if(std::strcmp(info_it->name.GetString(), "type") == 0){
				continue;
			}
			// TODO: Better way to get rapidjson value, currently only supports int float precision also
			if(info_it->value.IsString()){
				actor_components.at(component_key).lua_ref[info_it->name.GetString()] = info_it->value.GetString();
			}else if(info_it->value.IsBool()){
				actor_components.at(component_key).lua_ref[info_it->name.GetString()] = info_it->value.GetBool();
			}else if(info_it->value.IsInt()){
				actor_components.at(component_key).lua_ref[info_it->name.GetString()] = info_it->value.GetInt();
			}else if(info_it->value.IsFloat()){
				actor_components.at(component_key).lua_ref[info_it->name.GetString()] = info_it->value.GetFloat();
			}
		}
	}
}

ActorData::ActorData(const rapidjson::Value& obj){
	initActorData(obj);
}

Actor::Actor(){
	actorID = g_uuid;
	g_uuid++;
}

int Actor::GetID(){
	return actorID;
}

std::string Actor::GetName(){
	return data.actor_name;
}

luabridge::LuaRef Actor::GetComponentByKey(const std::string& key){
	if(data.actor_components.count(key)){
		if(data.actor_components.at(key).to_delete) return ComponentDB::GetNil();
		return data.actor_components.at(key).lua_ref;
	} 
	if(runtime_components_to_add.count(key)){
		if(runtime_components_to_add.at(key).to_delete) return ComponentDB::GetNil();
		return runtime_components_to_add.at(key).lua_ref;
	}
	
	return ComponentDB::GetNil();
}

//TODO: Make this O(1)
luabridge::LuaRef Actor::GetComponentByType(const std::string& type){
	auto componenet_it = data.actor_components.begin();
	auto to_add_it = runtime_components_to_add.begin();

	while(componenet_it != data.actor_components.end() && to_add_it != runtime_components_to_add.end()){
		if(componenet_it->first <= to_add_it->first){
			if(!componenet_it->second.to_delete && componenet_it->second.type == type){
				return componenet_it->second.lua_ref;
			}
			componenet_it++;
		}else{
			if(!to_add_it->second.to_delete && to_add_it->second.type == type){
				return to_add_it->second.lua_ref;
			}
			to_add_it++;
		}
	}

	while(componenet_it != data.actor_components.end()){
		if(!componenet_it->second.to_delete && componenet_it->second.type == type) return componenet_it->second.lua_ref;
		componenet_it++;
	}
	while(to_add_it != runtime_components_to_add.end()){
		if(!to_add_it->second.to_delete && to_add_it->second.type == type) return to_add_it->second.lua_ref;
		to_add_it++;
	}
	return ComponentDB::GetNil();
}

luabridge::LuaRef Actor::GetComponentsOfType(const std::string& type){
	luabridge::LuaRef table = luabridge::newTable(ComponentDB::lua_state);
	int idx = 1;

	auto component_it = data.actor_components.begin();
	auto to_add_it = runtime_components_to_add.begin();

	while(component_it != data.actor_components.end() && to_add_it != runtime_components_to_add.end()){
		if(component_it->first <= to_add_it->first){
			if(!component_it->second.to_delete && component_it->second.type == type){
				table[idx] = component_it->second.lua_ref;
				idx++;
			}
			component_it++;
		}else{
			if(!to_add_it->second.to_delete && to_add_it->second.type == type){
				table[idx] = to_add_it->second.lua_ref;
				idx++;
			}
			to_add_it++;
		}
	}

	while(component_it != data.actor_components.end()){
		if(!component_it->second.to_delete && component_it->second.type == type){
			table[idx] = component_it->second.lua_ref;
			idx++;
		}
		component_it++;
	}
	while(to_add_it != runtime_components_to_add.end()){
		if(!to_add_it->second.to_delete && to_add_it->second.type == type){
			table[idx] = to_add_it->second.lua_ref;
			idx++;
		}
		to_add_it++;
	}

	return table;
}

luabridge::LuaRef Actor::addRunTimeComponent(const std::string& type_name){
	std::string component_name = "r" + std::to_string(numRuntimeAddedComponents);
	numRuntimeAddedComponents++;

	luabridge::LuaRef new_component = ComponentDB::CreateComponent(type_name, component_name);
	new_component["actor"] = this;
	runtime_components_to_add.insert({component_name, actor_component(type_name, new_component)});
	return new_component;
}

void Actor::removeComponent(luabridge::LuaRef component_ref){
	component_ref["enabled"] = false;
	data.actor_components.at(component_ref["key"].cast<std::string>()).to_delete = true;
	components_to_remove.push_back(component_ref["key"].cast<std::string>());
	SceneDB::to_delete_refs.push_back(component_ref);
}

// TODO: these can  be made faster, no need to iterate through all components
// however, need to maintain execution by order of key
// possibly implement c++23 flatset, or just use a vector + bulk insert + sort

void Actor::OnCollisionEnter(Collision c){
	for(auto& [key, component] : data.actor_components){
        if(component.to_delete || !component.lua_ref["enabled"]) continue;
        if(component.lua_ref["OnCollisionEnter"].isFunction()){
            try {
                component.lua_ref["OnCollisionEnter"](component.lua_ref, c);
            } catch(const luabridge::LuaException& e) {
                EngineUtils::ReportError(data.actor_name, e);
            }
        }
    }
};

void Actor::OnCollisionExit(Collision c){
	for(auto& [key, component] : data.actor_components){
        if(component.to_delete || !component.lua_ref["enabled"]) continue;
        if(component.lua_ref["OnCollisionExit"].isFunction()){
            try {
                component.lua_ref["OnCollisionExit"](component.lua_ref, c);
            } catch(const luabridge::LuaException& e) {
                EngineUtils::ReportError(data.actor_name, e);
            }
        }
    }
};

void Actor::OnTriggerEnter(Collision c){
	for(auto& [key, component] : data.actor_components){
        if(component.to_delete || !component.lua_ref["enabled"]) continue;
        if(component.lua_ref["OnTriggerEnter"].isFunction()){
            try {
                component.lua_ref["OnTriggerEnter"](component.lua_ref, c);
            } catch(const luabridge::LuaException& e) {
                EngineUtils::ReportError(data.actor_name, e);
            }
        }
    }
};

void Actor::OnTriggerExit(Collision c){
	for(auto& [key, component] : data.actor_components){
        if(component.to_delete || !component.lua_ref["enabled"]) continue;
        if(component.lua_ref["OnTriggerExit"].isFunction()){
            try {
                component.lua_ref["OnTriggerExit"](component.lua_ref, c);
            } catch(const luabridge::LuaException& e) {
                EngineUtils::ReportError(data.actor_name, e);
            }
        }
    }
};


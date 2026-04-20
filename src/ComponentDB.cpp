#include "ComponentDB.h"
#include "rapidjson/document.h"
#include "EngineUtils.h"
#include "Actor.h"
#include "SceneDB.h"
#include "Helper.h"
#include "Input.h"
#include "TextDB.h"
#include "AudioDB.h"
#include "ImageDB.h"
#include "Renderer.h"
#include "Rigidbody.h"
#include "Raycast.h"
#include "Eventbus.h"
#include "ParticleSystem.h"

#include <filesystem>
#include <iostream>
#include <thread>

void ComponentDB::Init(){
    InitState();
    InitFunctions();
    InitComponents();

    // TODO: more generalizable way to group C++ components
    component_table.insert({"Rigidbody", luabridge::LuaRef(ComponentDB::lua_state, new Rigidbody())});
    component_table.insert({"ParticleSystem", luabridge::LuaRef(ComponentDB::lua_state, new ParticleSystem())});
}

void ComponentDB::InitState(){
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);
}

void ComponentDB::CppLog(const std::string& message){
    std::cout << message << "\n";
}

luabridge::LuaRef ComponentDB::GetNil(){
    return luabridge::LuaRef(ComponentDB::lua_state);
}
void ComponentDB::InitFunctions(){
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Debug")
        .addFunction("Log", ComponentDB::CppLog)
        .endNamespace()
    
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponentByType)
        .addFunction("GetComponents", &Actor::GetComponentsOfType)
        .addFunction("AddComponent", &Actor::addRunTimeComponent)
        .addFunction("RemoveComponent", &Actor::removeComponent)
        .endClass()

        .beginNamespace("Actor")
        .addFunction("Find", SceneDB::FindActor)
        .addFunction("FindAll", SceneDB::FindAllActors)
        .addFunction("Instantiate", SceneDB::InstantiateActor)
        .addFunction("Destroy", SceneDB::DestroyActor)
        .endNamespace()

        .beginNamespace("Text")
        .addFunction("Draw", TextDB::DrawText)
        .endNamespace()

        .beginNamespace("Audio")
        .addFunction("Play", AudioDB::PlayAudio)
        .addFunction("Halt", AudioDB::stopAudioChannel)
        .addFunction("SetVolume", AudioDB::setVolume)
        .endNamespace()

        .beginNamespace("Image")
        .addFunction("DrawUI", ImageDB::DrawUI)
        .addFunction("DrawUIEx", ImageDB::DrawUIEx)
        .addFunction("Draw", ImageDB::Draw)
        .addFunction("DrawEx", ImageDB::DrawEx)
        .addFunction("DrawPixel", ImageDB::DrawPixel)
        .endNamespace()

        .beginNamespace("Camera")
        .addFunction("SetPosition", Renderer::SetCameraPosition)
        .addFunction("GetPositionX", Renderer::GetCameraPositionX)
        .addFunction("GetPositionY", Renderer::GetCameraPositionY)
        .addFunction("SetZoom", Renderer::SetCameraZoom)
        .addFunction("GetZoom", Renderer::GetCameraZoom)
        .endNamespace()

        .beginNamespace("Scene")
        .addFunction("Load", SceneDB::stageNewScene)
        .addFunction("GetCurrent", SceneDB::GetCurrentScene)
        .addFunction("DontDestroy", SceneDB::ProtectActorOnSceneChange)
        .endNamespace()

        .beginNamespace("Application")
        .addFunction("Quit", static_cast<void(*)()>( [](){ std::exit(0); } ))
        .addFunction("Sleep", static_cast<void(*)(int dur_ms)>(
                [](int dur_ms){ std::this_thread::sleep_for(std::chrono::milliseconds(dur_ms));}))
        .addFunction("GetFrame", Helper::GetFrameNumber)
        .addFunction("OpenURL", EngineUtils::OpenURL)
        .endNamespace()

        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass()
            
        .beginNamespace("Input")
        .addFunction("GetKey", static_cast<bool(*)(const std::string&)>(&Input::GetKey))
        .addFunction("GetKeyDown", static_cast<bool(*)(const std::string&)>(&Input::GetKeyDown))
        .addFunction("GetKeyUp", static_cast<bool(*)(const std::string&)>(&Input::GetKeyUp))
        .addFunction("GetMousePosition", Input::GetMousePosition)
        .addFunction("GetMouseButton", Input::GetMouseButton)
        .addFunction("GetMouseButtonDown", Input::GetMouseButtonDown)
        .addFunction("GetMouseButtonUp", Input::GetMouseButtonUp)
        .addFunction("GetMouseScrollDelta", Input::GetMouseScrollData)
        .addFunction("HideCursor", Input::HideCursor)
        .addFunction("ShowCursor", Input::ShowCursor)
        .endNamespace()

        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*) (float, float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", static_cast<b2Vec2(*)(const b2Vec2*, const b2Vec2&)>([](const b2Vec2* a, const b2Vec2& b) {
            return b2Vec2(a->x + b.x, a->y + b.y);
        }))  
        .addFunction("__sub", static_cast<b2Vec2(*)(const b2Vec2*, const b2Vec2&)>([](const b2Vec2* a, const b2Vec2& b) {
            return b2Vec2(a->x - b.x, a->y - b.y);
        }))
        .addFunction("__mul", static_cast<b2Vec2(*)(const b2Vec2*, const float&)>([](const b2Vec2* a, const float& b) {
            return b2Vec2(a->x * b, a->y * b);
        }))
        .addStaticFunction("Distance", b2Distance)
        .addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
        .endClass()

        .beginClass<Rigidbody>("Rigidbody")
        .addConstructor<void(*) (void)>()
        .addProperty("x", &Rigidbody::initial_x)
        .addProperty("y", &Rigidbody::initial_y)
        .addProperty("body_type", &Rigidbody::body_type)
        .addProperty("precise", &Rigidbody::precise)
        .addProperty("gravity_scale", &Rigidbody::gravity_scale)
        .addProperty("density", &Rigidbody::density)
        .addProperty("angular_friction", &Rigidbody::angular_friction)
        .addProperty("rotation", &Rigidbody::rotation)
        .addProperty("has_collider", &Rigidbody::has_collider)
        .addProperty("has_trigger", &Rigidbody::has_trigger)
        .addProperty("actor", &Rigidbody::actor)
        .addProperty("enabled", &Rigidbody::enabled)
        .addProperty("key", &Rigidbody::key)
        .addProperty("width", &Rigidbody::width) 
        .addProperty("height", &Rigidbody::height)
        .addProperty("radius", &Rigidbody::radius)
        .addProperty("friction", &Rigidbody::friction)
        .addProperty("bounciness", &Rigidbody::bounciness)
        .addProperty("collider_type", &Rigidbody::collider_type)
        .addProperty("trigger_type", &Rigidbody::trigger_type)
        .addProperty("trigger_width", &Rigidbody::trigger_width)
        .addProperty("trigger_height", &Rigidbody::trigger_height)
        .addProperty("trigger_radius", &Rigidbody::trigger_radius)
        .addFunction("OnStart", &Rigidbody::OnStart)
        .addFunction("OnDestroy", &Rigidbody::OnDestroy)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("GetVelocity" ,&Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
        .endClass()

        .beginClass<Collision>("Collision")
        .addProperty("other", &Collision::other)
        .addProperty("point", &Collision::point)
        .addProperty("relative_velocity", &Collision::relative_velocity)
        .addProperty("normal", &Collision::normal)
        .endClass()

        .beginClass<HitResult>("HitResult")
        .addProperty("actor", &HitResult::actor)
        .addProperty("point", &HitResult::point)
        .addProperty("is_trigger", &HitResult::is_trigger)
        .addProperty("normal", &HitResult::normal)
        .endClass()

        .beginNamespace("Physics")
        .addFunction("Raycast", Rigidbody::FireRaycast)
        .addFunction("RaycastAll", Rigidbody::FireAllRaycast)
        .endNamespace()

        .beginNamespace("Event")
        .addFunction("Publish", Eventbus::Publish)
        .addFunction("Subscribe", Eventbus::Subscribe)
        .addFunction("Unsubscribe", Eventbus::Unsubscribe)
        .endNamespace()
        
        .beginClass<ParticleSystem>("ParticleSystem")
        .addProperty("key", &ParticleSystem::key)
        .addProperty("actor", &ParticleSystem::actor)
        .addProperty("enabled", &ParticleSystem::enabled)
        .addProperty("x", &ParticleSystem::starting_x)
        .addProperty("y", &ParticleSystem::starting_y)
        .addProperty("frames_between_bursts", &ParticleSystem::frames_between_bursts)
        .addProperty("burst_quantity", &ParticleSystem::burst_quantity)
        .addProperty("start_scale_min", &ParticleSystem::start_scale_min)
        .addProperty("start_scale_max", &ParticleSystem::start_scale_max)
        .addProperty("rotation_min", &ParticleSystem::rotation_min)
        .addProperty("rotation_max", &ParticleSystem::rotation_max)
        .addProperty("start_color_r", &ParticleSystem::start_color_r)
        .addProperty("start_color_g", &ParticleSystem::start_color_g)
        .addProperty("start_color_b", &ParticleSystem::start_color_b)
        .addProperty("start_color_a", &ParticleSystem::start_color_a)
        .addProperty("emit_radius_min", &ParticleSystem::emit_radius_min)
        .addProperty("emit_radius_max", &ParticleSystem::emit_radius_max)
        .addProperty("emit_angle_min", &ParticleSystem::emit_angle_min)
        .addProperty("emit_angle_max", &ParticleSystem::emit_angle_max)
        .addProperty("image", &ParticleSystem::image)
        .addProperty("sorting_order", &ParticleSystem::sorting_order)
        .addProperty("duration_frames", &ParticleSystem::duration_frames)
        .addProperty("start_speed_min", &ParticleSystem::start_speed_min)
        .addProperty("start_speed_max", &ParticleSystem::start_speed_max)
        .addProperty("rotation_speed_min", &ParticleSystem::rotation_speed_min)
        .addProperty("rotation_speed_max", &ParticleSystem::rotation_speed_max)
        .addProperty("gravity_scale_x", &ParticleSystem::gravity_scale_x)
        .addProperty("gravity_scale_y", &ParticleSystem::gravity_scale_y)
        .addProperty("drag_factor", &ParticleSystem::drag_factor)
        .addProperty("angular_drag_factor", &ParticleSystem::angular_drag_factor)
        .addProperty("end_scale", &ParticleSystem::end_scale)
        .addProperty("end_color_r", &ParticleSystem::end_color_r)
        .addProperty("end_color_g", &ParticleSystem::end_color_g)
        .addProperty("end_color_b", &ParticleSystem::end_color_b)
        .addProperty("end_color_a", &ParticleSystem::end_color_a)
        .addFunction("OnStart", &ParticleSystem::OnStart)
        .addFunction("OnUpdate", &ParticleSystem::OnUpdate)
        .addFunction("Stop", &ParticleSystem::Stop)
        .addFunction("Play", &ParticleSystem::Play)
        .addFunction("Burst", &ParticleSystem::Burst)
        .endClass();
};
   

void ComponentDB::InitComponents(){
    if(!std::filesystem::exists(COMPONENT_FILEPATH)) return;

    for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(COMPONENT_FILEPATH)){
        std::string component_name = entry.path().stem().string();
        if(luaL_dofile(lua_state, entry.path().string().c_str()) != LUA_OK){
            std::cout << "problem with lua file " << component_name;
            exit(0);
        }
        component_table.insert({component_name, luabridge::getGlobal(lua_state, component_name.c_str())});
    }
}

void ComponentDB::EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table){
    luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
    
    new_metatable["__index"] = parent_table;

    instance_table.push(lua_state);
    new_metatable.push(lua_state);
    lua_setmetatable(lua_state, -2);
    lua_pop(lua_state, 1);
}

luabridge::LuaRef ComponentDB::CreateComponent(const std::string& type_name, const std::string& component_key){
	if(!component_table.count(type_name)){
		std::cout << "error: failed to locate component " << type_name;
		exit(0);
	}
	luabridge::LuaRef& component_type_instance = component_table.at(type_name);
	luabridge::LuaRef new_component = luabridge::newTable(lua_state);

    // TODO: more generalizable way to group C++ components
	if(type_name == "Rigidbody"){
		Rigidbody* new_rigid_body = nullptr;
		if(!component_type_instance.isNil()){
			Rigidbody* parent_component = component_type_instance.cast<Rigidbody*>();
			new_rigid_body = new Rigidbody(*parent_component);
		}
    	new_component = luabridge::LuaRef(lua_state, new_rigid_body);
	}else if(type_name == "ParticleSystem"){
        ParticleSystem* new_particle_system = nullptr;
		if(!component_type_instance.isNil()){
			ParticleSystem* parent_component = component_type_instance.cast<ParticleSystem*>();
			new_particle_system = new ParticleSystem(*parent_component);
		}
    	new_component = luabridge::LuaRef(lua_state, new_particle_system);
    }
    else{
		EstablishInheritance(new_component, component_type_instance);
	}

	new_component["key"] = component_key;
	new_component["enabled"] = true;
	return new_component;
}
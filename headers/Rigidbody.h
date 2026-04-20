#ifndef RIGID_BODY
#define RIGID_BODY

#include "box2d/box2d.h"

#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "Actor.h"
#include "ContactListener.h"
#include "Raycast.h"

#include <memory>
#include <string>


class Rigidbody{
private:
    static inline int32 VELOCITY_ITERATIONS = 8;
    static inline int32 POSITION_ITERATIONS = 3;
    static inline float TIME_STEP = 1.0f/60.0f;

    static inline bool world_initialized = false;
    static inline std::unique_ptr<ContactListener> contact_listener;
    static inline std::unique_ptr<b2World> physics_world;
    static inline std::unique_ptr<Raycast> raycast;
    static inline std::unique_ptr<RaycastAll> raycastAll;

    b2Body* body = nullptr;
    b2Fixture* fixture = nullptr;
    b2Fixture* trigger_fixture = nullptr;

    b2Vec2 pending_force       = {0.0f, 0.0f};
    b2Vec2 pending_velocity    = {0.0f, 0.0f};
    float  pending_angular_vel = 0.0f;
public:
    std::string body_type = "dynamic";
    std::string collider_type = "box";
    std::string trigger_type = "box";
    std::string key; // Needed for lua component to function
    
    Actor* actor = nullptr;  // Needed for lua component to function

    float initial_x = 0.0f;
    float initial_y = 0.0f;

    float gravity_scale = 1.0f;
    float density = 1.0f;
    float angular_friction = 0.3f;
    float rotation = 0.0f;

    float width = 1.0f;
    float height = 1.0f;
    float radius = 0.5f;

    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;

    float friction = 0.3f;
    float bounciness = 0.3f;

    bool precise = true;
    bool has_collider = true;
    bool has_trigger = true;

    bool enabled = false; // Needed for lua component to function

    Rigidbody();
    
    static void Advance();
    static void DestroyRigidBody(Rigidbody& rigid_body);

    void OnStart();  
    void OnDestroy();
    
    void AddForce(const b2Vec2& vector2);
    void SetVelocity(const b2Vec2& vector2);
    void SetPosition(const b2Vec2& vector2);
    void SetRotation(float degrees_clockwise);
    void SetAngularVelocity(float degrees_clockwise);
    void SetGravityScale(float g_scale);
    void SetUpDirection(b2Vec2 vector2);
    void SetRightDirection(b2Vec2 vector2);

    b2Vec2 GetPosition() const;
    float GetRotation() const;
    b2Vec2 GetVelocity() const;
    float GetAngularVelocity() const;
    float GetGravityScale() const;
    b2Vec2 GetUpDirection() const;
    b2Vec2 GetRightDirection() const;

    static luabridge::LuaRef FireRaycast(const b2Vec2& pos, const b2Vec2& dir, float dist);
    static luabridge::LuaRef FireAllRaycast(const b2Vec2& pos, const b2Vec2& dir, float dist);
};




#endif

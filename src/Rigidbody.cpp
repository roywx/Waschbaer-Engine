#include "Rigidbody.h"
#include <algorithm>

#define CATEGORY_NULL 0x0000
#define CATEGORY_COLLIDER 0x0001
#define CATEGORY_TRIGGER  0x0002
#define CATEGORY_PHANTOM  0x0004

Rigidbody::Rigidbody(){}

void Rigidbody::OnStart(){
    if(!world_initialized){
        physics_world = std::make_unique<b2World>(b2Vec2(0.0f, 9.8f));
        contact_listener =  std::make_unique<ContactListener>();
        physics_world->SetContactListener(contact_listener.get());
        world_initialized = true;
        raycast = std::make_unique<Raycast>();
        raycastAll = std::make_unique<RaycastAll>();
    }

    b2BodyDef body_def;

    if      (body_type == "dynamic")   body_def.type = b2_dynamicBody;
    else if (body_type == "static")    body_def.type = b2_staticBody;
    else if (body_type == "kinematic") body_def.type = b2_kinematicBody;
    
    body_def.position.Set(initial_x, initial_y);
    body_def.angle = rotation * (b2_pi/180.0f);
    body_def.bullet = precise;
    body_def.gravityScale = gravity_scale;
    body_def.angularDamping = angular_friction;

    body = physics_world->CreateBody(&body_def);

    // phamton sensor to make bodies move if neither collider nor trigger is present
    if(!has_collider && !has_trigger){
        b2PolygonShape phamton_shape;
        phamton_shape.SetAsBox(width * 0.5f, height * 0.5f);

        b2FixtureDef phamton_fixture_def;
        phamton_fixture_def.shape = &phamton_shape;
        phamton_fixture_def.density = density;
        phamton_fixture_def.isSensor = true;

        phamton_fixture_def.filter.categoryBits = CATEGORY_PHANTOM;
        phamton_fixture_def.filter.maskBits = CATEGORY_NULL;

        body->CreateFixture(&phamton_fixture_def);
        return;
    }

    if(has_collider){
        b2FixtureDef fixture_def;
        fixture_def.density = density;
        fixture_def.friction = friction;
        fixture_def.restitution = bounciness;
        fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);

        fixture_def.filter.categoryBits = CATEGORY_COLLIDER;
        fixture_def.filter.maskBits     = CATEGORY_COLLIDER;

        if(collider_type == "box"){
            b2PolygonShape shape;
            shape.SetAsBox(width * 0.5f, height * 0.5f);
            fixture_def.shape = &shape;
            fixture = body->CreateFixture(&fixture_def);
        }else if(collider_type == "circle"){
            b2CircleShape shape;
            shape.m_radius = radius;
            fixture_def.shape = &shape;
            fixture = body->CreateFixture(&fixture_def);
        }
    }

    if(has_trigger){
        b2FixtureDef trigger_fixture_def;
        trigger_fixture_def.isSensor = true;
        trigger_fixture_def.density = density;
        trigger_fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
        
        trigger_fixture_def.filter.categoryBits = CATEGORY_TRIGGER;
        trigger_fixture_def.filter.maskBits     = CATEGORY_TRIGGER;

        if(trigger_type == "box"){
            b2PolygonShape shape;
            shape.SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
            trigger_fixture_def.shape = &shape;
            trigger_fixture = body->CreateFixture(&trigger_fixture_def);
        }else if(trigger_type == "circle"){
            b2CircleShape shape;
            shape.m_radius = trigger_radius;
            trigger_fixture_def.shape = &shape;
            trigger_fixture = body->CreateFixture(&trigger_fixture_def);
        }
    }

    body->SetLinearVelocity(pending_velocity);
    body->SetAngularVelocity(pending_angular_vel * (b2_pi / 180.0f));
    body->ApplyForceToCenter(pending_force, true);
}

void Rigidbody::Advance(){
    if(!physics_world) return;
    physics_world->Step(TIME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
}

void Rigidbody::OnDestroy(){
    if(!physics_world) return;
    physics_world->DestroyBody(body);
}
void Rigidbody::DestroyRigidBody(Rigidbody& rigid_body){
    if(!physics_world) return;
    physics_world->DestroyBody(rigid_body.body);
}

void Rigidbody::AddForce(const b2Vec2& vector2){
    if(body == nullptr){
        pending_force.x += vector2.x;
        pending_force.y += vector2.y;
        return;
    }
    body->ApplyForceToCenter(vector2, true);
}

void Rigidbody::SetVelocity(const b2Vec2& vector2){
    if(body == nullptr){
        pending_velocity = vector2;
        return;
    }
    body->SetLinearVelocity(vector2);
}
void Rigidbody::SetPosition(const b2Vec2& vector2){
    if(body == nullptr){
        initial_x = vector2.x;
        initial_y = vector2.y;
        return;    
    }
    body->SetTransform(vector2, body->GetAngle());
}
void Rigidbody::SetRotation(float degrees_clockwise){
    if(body == nullptr){
       rotation = degrees_clockwise;  
       return;  
    }
    float radians = degrees_clockwise * (b2_pi / 180.0f);
    body->SetTransform(body->GetPosition(), radians);
}   
void Rigidbody::SetAngularVelocity(float degrees_clockwise){
     if(body == nullptr){
        pending_angular_vel = degrees_clockwise;
        return;
    }
    float radians = degrees_clockwise * (b2_pi / 180.0f);
    body->SetAngularVelocity(radians);
}
void Rigidbody::SetGravityScale(float g_scale){
     if(body == nullptr){
       gravity_scale = g_scale;  
       return;  
    }
    body->SetGravityScale(g_scale);
}
void Rigidbody::SetUpDirection(b2Vec2 vector2){
    vector2.Normalize();
    float angle = glm::atan(vector2.x, -vector2.y) ;
    if(body == nullptr){
        rotation = angle * (180.0f / b2_pi);
        return;
    }
    body->SetTransform(body->GetPosition(), angle);
}
void Rigidbody::SetRightDirection(b2Vec2 vector2){
    vector2.Normalize();
    float angle = glm::atan(vector2.x, -vector2.y) - (b2_pi/2.0f);
    if(body == nullptr){
        rotation = angle * (180.0f / b2_pi);
        return;
    }
    body->SetTransform(body->GetPosition(), angle);
}

float Rigidbody::GetRotation() const {
    if (!body) return rotation;
    return body->GetAngle() * (180.0f / b2_pi);
}

b2Vec2 Rigidbody::GetPosition() const {
    if(!body) return b2Vec2(initial_x, initial_y);
    return b2Vec2( body->GetPosition().x,  body->GetPosition().y);
}

b2Vec2 Rigidbody::GetVelocity() const {
    return body->GetLinearVelocity(); 
}

float Rigidbody::GetAngularVelocity() const { 
    return (body->GetAngularVelocity() * (180.0f/b2_pi)); 
}

float Rigidbody::GetGravityScale() const { 
    return body->GetGravityScale(); 
}

b2Vec2 Rigidbody::GetUpDirection() const {
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
    result.Normalize();
    return result;
}

b2Vec2 Rigidbody::GetRightDirection() const {
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
    result.Normalize();
    return result;
}

luabridge::LuaRef Rigidbody::FireRaycast(const b2Vec2& pos, const b2Vec2& dir, float dist){
    if(!physics_world) return ComponentDB::GetNil();

    raycast->result.hit = false;

    physics_world->RayCast(raycast.get(), pos, pos + (dist * dir));
    
    if(!raycast->result.hit) return ComponentDB::GetNil();
    return luabridge::LuaRef(ComponentDB::lua_state, raycast->result);
}

luabridge::LuaRef Rigidbody::FireAllRaycast(const b2Vec2& pos, const b2Vec2& dir, float dist){
    luabridge::LuaRef table = luabridge::newTable(ComponentDB::lua_state);

    if(!physics_world) return table;

    raycastAll->results.clear();
    physics_world->RayCast(raycastAll.get(), pos, pos + (dist * dir));
    
    std::sort(raycastAll->results.begin(), raycastAll->results.end(), 
        [](const HitResult& a, const HitResult& b){
            return a.fraction < b.fraction;
    });
    
    for(size_t i = 0; i < raycastAll->results.size(); i++){
        table[i + 1] = raycastAll->results[i];
    }
    return table;
}
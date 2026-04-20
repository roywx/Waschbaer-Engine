#include "ContactListener.h"
#include "Actor.h"

void ContactListener::BeginContact(b2Contact* contact){
    b2Fixture* fixA = contact->GetFixtureA();
    b2Fixture* fixB = contact->GetFixtureB();
    
    if(fixA->GetUserData().pointer == 0 || fixB->GetUserData().pointer == 0) return;

    Actor* actor_a = reinterpret_cast<Actor*>(fixA->GetUserData().pointer);
    Actor* actor_b = reinterpret_cast<Actor*>(fixB->GetUserData().pointer);

    b2Vec2 relative_velocity = fixA->GetBody()->GetLinearVelocity() - fixB->GetBody()->GetLinearVelocity();

    Collision collision(actor_b, SENTINEL_VEC, relative_velocity, SENTINEL_VEC);

    if(fixA->IsSensor() && fixB->IsSensor()){
        actor_a->OnTriggerEnter(collision);
        collision.other = actor_a;
        actor_b->OnTriggerEnter(collision);
    }else if(!fixA->IsSensor() && !fixB->IsSensor()){
        b2WorldManifold world_manifold;
        contact->GetWorldManifold(&world_manifold);

        collision.point = world_manifold.points[0];
        collision.normal = world_manifold.normal;

        actor_a->OnCollisionEnter(collision);
        collision.other = actor_a;
        actor_b->OnCollisionEnter(collision);
    }
}

void ContactListener::EndContact(b2Contact* contact){
    b2Fixture* fixA = contact->GetFixtureA();
    b2Fixture* fixB = contact->GetFixtureB();

    if(fixA->GetUserData().pointer == 0 || fixB->GetUserData().pointer == 0) return;

    Actor* actor_a = reinterpret_cast<Actor*>(fixA->GetUserData().pointer);
    Actor* actor_b = reinterpret_cast<Actor*>(fixB->GetUserData().pointer);

    b2Vec2 relative_velocity = fixA->GetBody()->GetLinearVelocity() - fixB->GetBody()->GetLinearVelocity();

    Collision collision(actor_b, SENTINEL_VEC, relative_velocity, SENTINEL_VEC);

    if(fixA->IsSensor() && fixB->IsSensor()){
        actor_a->OnTriggerExit(collision);
        collision.other = actor_a;
        actor_b->OnTriggerExit(collision);
    }else if(!fixA->IsSensor() && !fixB->IsSensor()){
        actor_a->OnCollisionExit(collision);
        collision.other = actor_a;
        actor_b->OnCollisionExit(collision);
    }
}
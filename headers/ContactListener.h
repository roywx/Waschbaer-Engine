#ifndef CONTACT_LISTENER
#define CONTACT_LISTENER

#include "box2d/box2d.h"

class Actor;

struct Collision{
public:
    Actor* other;
    b2Vec2 point;
    b2Vec2 relative_velocity;
    b2Vec2 normal;

    Collision(Actor* pa, b2Vec2 pp, b2Vec2 prv, b2Vec2 pn) : 
        other(pa), point(pp), relative_velocity(prv), normal(pn){};
};

class ContactListener : public b2ContactListener{
private:
    b2Vec2 SENTINEL_VEC = b2Vec2(-999.0f, -999.0f);
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
};

#endif
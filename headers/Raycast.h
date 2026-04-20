#ifndef RAYCAST_H
#define RAYCAST_H

#include "box2d/box2d.h"
#include "Actor.h"
#include <vector>

struct HitResult{
public:
    Actor* actor;
    b2Vec2 point;
    b2Vec2 normal;
    float fraction; // Helper variable not visible to scripting layer:
    bool is_trigger;
    bool hit;  // Helper variable not visible to scripting layer:

};

class Raycast : public b2RayCastCallback{
    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
public:
    HitResult result;
};

class RaycastAll : public b2RayCastCallback{
    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
public:
    std::vector<HitResult> results;
};

#endif
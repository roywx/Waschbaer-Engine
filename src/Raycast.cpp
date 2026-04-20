#include "Raycast.h"

float Raycast::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction){
    if(fixture->GetUserData().pointer == 0) return -1;

    result.actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer); 
    result.point = point;
    result.normal = normal;
    result.is_trigger = fixture->IsSensor();

    result.hit = true;
    result.fraction = fraction;

    return fraction;
}

float RaycastAll::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction){
    if(fixture->GetUserData().pointer == 0) return -1;

    HitResult new_result;
    new_result.actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer); 
    new_result.point = point;
    new_result.normal = normal;
    new_result.is_trigger = fixture->IsSensor();

    new_result.hit = true;
    new_result.fraction = fraction;

    results.push_back(new_result);

    return 1;
}
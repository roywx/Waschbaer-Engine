#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "Helper.h"
#include "Actor.h"
#include "glm/glm.hpp"

#include <vector>
#include <queue>

class ParticleSystem{
private:
    struct ParticleDrawData {
        float x, y, rot, scale;
        float r, g, b, a;
        bool active;
    };

    int GetFreeIndex();
    bool emission_allowed = true;

    std::vector<ParticleDrawData> draw_data;
    void SimulateRange(int start, int end);
    void SubmitDrawCalls();
    void RebuildFreeList();
public:

    bool enabled = false; // Needed for lua component to function
    int local_frame = 0; 
    int frames_between_bursts = 1;
    int burst_quantity = 1;
    int duration_frames = 300;
    int sorting_order = 9999;
    float start_color_r = 255.0f;
    float start_color_g = 255.0f;
    float start_color_b = 255.0f;
    float start_color_a = 255.0f;
    float resolved_end_color_r;
    float resolved_end_color_g;
    float resolved_end_color_b;
    float resolved_end_color_a;
    float starting_x = 0.0f;
    float starting_y = 0.0f;
    float emit_angle_min = 0.0f;
    float emit_angle_max = 360.f;
    float emit_radius_min = 0.0f;
    float emit_radius_max = 0.5f;
    float rotation_min = 0.0f;
    float rotation_max = 0.0f;
    float start_scale_min = 1.0f;
    float start_scale_max = 1.0f;
    float start_speed_min = 0.0f;
    float start_speed_max = 0.0f;
    float rotation_speed_min = 0.0f;
    float rotation_speed_max = 0.0f;
    float drag_factor = 1.0f;
    float angular_drag_factor = 1.0f;
    float gravity_scale_x = 0.0f;
    float gravity_scale_y = 0.0f;
    
    float end_color_r = NAN;
    float end_color_g = NAN;
    float end_color_b = NAN;
    float end_color_a = NAN;
    float end_scale = NAN;

    Actor* actor = nullptr;  // Needed for lua component to function
    std::string key; // Needed for lua component to function
    std::string image = "";

    RandomEngine emit_angle_distribution;
    RandomEngine emit_radius_distribution;
    RandomEngine rotation_distribution;
    RandomEngine scale_distribution;
    RandomEngine speed_distribution;
    RandomEngine rotation_speed_distribution;

    std::queue<int> free_list;
    
    std::vector<char> is_actives;
    std::vector<int> start_frames;
    std::vector<float> initial_scales;
    std::vector<float> scales;
    std::vector<float> rots;
    std::vector<float> rot_velocities;
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;
    std::vector<glm::ivec4> colors;

    void OnStart();
    void OnUpdate();
    void Stop();
    void Play();
    void Burst();

    static void Init();
};

#endif
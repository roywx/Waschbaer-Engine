#include "ParticleSystem.h"
#include "ImageDB.h"
#include "ThreadPool.h"

int ParticleSystem::GetFreeIndex() {
    int i = positions.size();

    if (!free_list.empty()){
        i = free_list.front();
        free_list.pop();
        is_actives[i] = 1;
        return i;
    }

    positions.push_back({});
    start_frames.push_back({});
    initial_scales.push_back({});
    scales.push_back({});
    rots.push_back({});
    velocities.push_back({});
    rot_velocities.push_back({});
    colors.push_back({});
    is_actives.push_back(true);
    draw_data.push_back({});
    return i;
}

void ParticleSystem::Init(){
    ImageDB::CreateDefaultParticleTextureWithName("");
}

void ParticleSystem::OnStart(){
    emit_angle_distribution = RandomEngine(emit_angle_min, emit_angle_max, 298);
    emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);
  
    rotation_distribution = RandomEngine(rotation_min, rotation_max, 440);
    scale_distribution = RandomEngine(start_scale_min, start_scale_max, 494);
   
    speed_distribution = RandomEngine(start_speed_min, start_speed_max, 498);
    rotation_speed_distribution = RandomEngine(rotation_speed_min, rotation_speed_max, 305);

    if(frames_between_bursts <= 0) frames_between_bursts = 1;
    if(burst_quantity <= 0) burst_quantity = 1;
    if(duration_frames <= 0) duration_frames = 1;

    resolved_end_color_r = std::isnan(end_color_r) ? start_color_r : end_color_r;
    resolved_end_color_g = std::isnan(end_color_g) ? start_color_g : end_color_g;
    resolved_end_color_b = std::isnan(end_color_b) ? start_color_b : end_color_b;
    resolved_end_color_a = std::isnan(end_color_a) ? start_color_a : end_color_a;
}

void ParticleSystem::OnUpdate(){            
    if(local_frame % frames_between_bursts == 0 && emission_allowed){
        Burst();
    }
    int count = (int) positions.size();
    
    ThreadPool::ParallelFor(count, [this](int start, int end) {
        SimulateRange(start, end);
    });

    RebuildFreeList();
    SubmitDrawCalls();
    local_frame++;
}   

void ParticleSystem::SimulateRange(int start, int end) { 
    for (int i = start; i < end; i++) {
        if (is_actives[i] == 0) {
            draw_data[i].active = false;
            continue;
        }
 
        int alive = local_frame - start_frames[i];
        if (alive >= duration_frames) {
            // TODO: Profile this for false sharing
            is_actives[i] = 0;
            draw_data[i].active = false;
            continue;
        }
 
        velocities[i].x += gravity_scale_x;
        velocities[i].y += gravity_scale_y;
        velocities[i] *= drag_factor;
        rot_velocities[i] *= angular_drag_factor;
        positions[i].x += velocities[i].x;
        positions[i].y += velocities[i].y;
        rots[i] += rot_velocities[i];
 
        float t = static_cast<float>(alive) / duration_frames;
 
        float target_scale = !std::isnan(end_scale) ? end_scale : initial_scales[i];
        scales[i] = glm::mix(initial_scales[i], target_scale, t);
 
        colors[i].r = static_cast<int>(glm::mix(start_color_r, resolved_end_color_r, t));
        colors[i].g = static_cast<int>(glm::mix(start_color_g, resolved_end_color_g, t));
        colors[i].b = static_cast<int>(glm::mix(start_color_b, resolved_end_color_b, t));
        colors[i].a = static_cast<int>(glm::mix(start_color_a, resolved_end_color_a, t));
 
        draw_data[i] = {
            positions[i].x, positions[i].y,
            rots[i], scales[i],
            static_cast<float>(colors[i].r), static_cast<float>(colors[i].g),
            static_cast<float>(colors[i].b), static_cast<float>(colors[i].a),
            true
        };
    }
}

void ParticleSystem::RebuildFreeList() {
    std::queue<int> empty;
    std::swap(free_list, empty);

    int count = (int)is_actives.size();
    for (int i = 0; i < count; i++) {
        if (is_actives[i] == 0) {
            free_list.push(i);
        }
    }
}

void ParticleSystem::SubmitDrawCalls() {
    int count = (int) draw_data.size();
    for (int i = 0; i < count; i++) {
        if (!draw_data[i].active) continue;
 
        ImageDB::DrawEx(image,
            draw_data[i].x, draw_data[i].y,
            draw_data[i].rot, draw_data[i].scale, draw_data[i].scale,
            0.5f, 0.5f,
            draw_data[i].r, draw_data[i].g, draw_data[i].b, draw_data[i].a,
            sorting_order);
    }
}

void ParticleSystem::Stop(){
    emission_allowed = false;
}

void ParticleSystem::Play(){
    emission_allowed = true;
}

void ParticleSystem::Burst(){
    for(int i = 0; i < burst_quantity; i++){
        int index = GetFreeIndex();

        // Emission shape
        float r = emit_radius_distribution.Sample();
        float theta = glm::radians(emit_angle_distribution.Sample());
        float cos_theta = glm::cos(theta);
        float sin_theta = glm::sin(theta);

        start_frames[index] = local_frame;
        positions[index] = {r * cos_theta + starting_x, r * sin_theta + starting_y};
        initial_scales[index] = scale_distribution.Sample();
        scales[index] = initial_scales[index];
        rots[index] = rotation_distribution.Sample();
        // Emission velocity
        float speed = speed_distribution.Sample();
        velocities[index] = {cos_theta * speed, sin_theta * speed};
        rot_velocities[index] = rotation_speed_distribution.Sample();
    }
}
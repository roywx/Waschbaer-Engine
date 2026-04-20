#include "Renderer.h"

#include <memory>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <cstdlib>

#include "EngineUtils.h"
#include "rapidjson/document.h"
#include "Helper.h"
#include "ThreadPool.h"

void Renderer::init(){
    SDL_Init(SDL_INIT_VIDEO);

    if(std::filesystem::exists("resources/rendering.config")){
        rapidjson::Document d;
        EngineUtils::ReadJsonFile("resources/rendering.config", d);
        
        EngineUtils::json_try_get_value(d, "x_resolution", Renderer::RESOLUTION.x);
        EngineUtils::json_try_get_value(d, "y_resolution", Renderer::RESOLUTION.y);
        EngineUtils::json_try_get_value(d, "clear_color_r", Renderer::clear_color.r);
        EngineUtils::json_try_get_value(d, "clear_color_g", Renderer::clear_color.g);
        EngineUtils::json_try_get_value(d, "clear_color_b", Renderer::clear_color.b);
        EngineUtils::json_try_get_value(d, "game_title", Renderer::game_title);
        EngineUtils::json_try_get_value(d, "cam_offset_x", Renderer::cam_offset.x);
        EngineUtils::json_try_get_value(d, "cam_offset_y", Renderer::cam_offset.y);
        EngineUtils::json_try_get_value(d, "zoom_factor", Renderer::zoom_factor);
    }
    inv_zoom = 1.0f / zoom_factor;
    SCREEN_X =  RESOLUTION.x/(zoom_factor * 2);
    SCREEN_Y =  RESOLUTION.y/(zoom_factor * 2);
    
    Renderer::window = Helper::SDL_CreateWindow(game_title.c_str(), 100, 100, 
                        Renderer::RESOLUTION.x, Renderer::RESOLUTION.y, SDL_WINDOW_SHOWN);
    Renderer::sdl_renderer = Helper::SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    SDL_RenderSetScale(sdl_renderer, Renderer::zoom_factor, Renderer::zoom_factor);
}

void Renderer::renderAllTexts(int frame_idx){

    for(const TextRequest& rq  : render_frames[frame_idx].text_requests){
        SDL_Surface* text_surface = TTF_RenderText_Solid(rq.font, rq.text.c_str(), rq.color);
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(sdl_renderer, text_surface);

        float w, h;
        Helper::SDL_QueryTexture(text_texture, &w, &h);
        SDL_FRect dst{static_cast<float>(rq.x) * inv_zoom, static_cast<float>(rq.y) * inv_zoom, w * inv_zoom, h * inv_zoom};

        Helper::SDL_RenderCopy(sdl_renderer, text_texture, nullptr, &dst);
        
        SDL_FreeSurface(text_surface);
        SDL_DestroyTexture(text_texture);
    }
    render_frames[frame_idx].text_requests.clear();
}

void Renderer::renderAllUI(int frame_idx){
    stable_sort(render_frames[frame_idx].UI_requests.begin(), render_frames[frame_idx].UI_requests.end(), [](const UIRequest& a, const UIRequest& b) {
        return a.sorting_order < b.sorting_order;
    });

    for(const UIRequest& rq : render_frames[frame_idx].UI_requests){
        float w, h;
        Helper::SDL_QueryTexture(rq.texture, &w, &h);
        SDL_SetTextureColorMod(rq.texture, rq.r, rq.g, rq.b);
        SDL_SetTextureAlphaMod(rq.texture, rq.a);
        SDL_FRect dst{static_cast<float>(rq.x) * inv_zoom, static_cast<float>(rq.y) * inv_zoom, w * inv_zoom, h * inv_zoom};
        Helper::SDL_RenderCopy(sdl_renderer, rq.texture, nullptr, &dst);
    }   

    render_frames[frame_idx].UI_requests.clear();
}

struct ResolvedRequest {
    float x_pos, y_pos, w, h;
    float pivot_x, pivot_y;
    SDL_RendererFlip flip;
    bool visible;
};

void Renderer::renderAllImages(int frame_idx){
    auto& reqs = render_frames[frame_idx].image_requests;
    int n = (int)reqs.size();
    if (n == 0) return;

    std::vector<ResolvedRequest> resolved(n);

    const float screen_w = RESOLUTION.x * inv_zoom;
    const float screen_h = RESOLUTION.y * inv_zoom;

    ThreadPool::ParallelFor(n, [&](int start, int end){
        for (int i = start; i < end; i++){
            const ImageRequest& rq = reqs[i];
            ResolvedRequest& r = resolved[i];

            r.w = rq.cached_w * glm::abs(rq.scale_x);
            r.h = rq.cached_h * glm::abs(rq.scale_y);
            r.pivot_x = rq.pivot_x * r.w;
            r.pivot_y = rq.pivot_y * r.h;
            r.x_pos = (rq.x - cam_pos.x) * PIXEL_DIST + SCREEN_X - r.pivot_x;
            r.y_pos = (rq.y - cam_pos.y) * PIXEL_DIST + SCREEN_Y - r.pivot_y;

            r.visible = !(r.x_pos + r.w < 0 || r.y_pos + r.h < 0
                       || r.x_pos > screen_w || r.y_pos > screen_h);

            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (rq.scale_y < 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
            if (rq.scale_x < 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
            r.flip = flip;
        }
    });

    // Cheaper to build and sort an index array 
    std::vector<int> order;
    order.reserve(n);
    for (int i = 0; i < n; i++) if (resolved[i].visible) order.push_back(i);

    std::stable_sort(order.begin(), order.end(), [&](int a, int b){
        return reqs[a].sorting_order < reqs[b].sorting_order;
    });

    for (int i : order){
        const ImageRequest& rq = reqs[i];
        const ResolvedRequest& r = resolved[i];

        SDL_FRect dst{r.x_pos, r.y_pos, r.w, r.h};
        SDL_FPoint pivot{r.pivot_x, r.pivot_y};

        SDL_SetTextureColorMod(rq.texture, rq.r, rq.g, rq.b);
        SDL_SetTextureAlphaMod(rq.texture, rq.a);
        Helper::SDL_RenderCopyEx(sdl_renderer, rq.texture,
                                 nullptr, &dst, rq.rot_deg, &pivot, r.flip);
    }

    reqs.clear();
}

void Renderer::renderAllPixels(int frame_idx){

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    for(const PixelRequest& rq : render_frames[frame_idx].pixel_requests){
        SDL_SetRenderDrawColor(sdl_renderer, rq.r, rq.g, rq.b, rq.a);
        SDL_RenderDrawPoint(sdl_renderer, rq.x, rq.y);        
    }
    render_frames[frame_idx].pixel_requests.clear();
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);
}

void Renderer::SetCameraPosition(float x, float y){
    cam_pos.x = x;
    cam_pos.y = y;
}

float Renderer::GetCameraPositionX(){
    return cam_pos.x;
}

float Renderer::GetCameraPositionY(){
    return cam_pos.y;
}

void Renderer::SetCameraZoom(float zoom){
    SDL_RenderSetScale(sdl_renderer, zoom, zoom);
    zoom_factor = zoom;
    inv_zoom = 1.0f / zoom_factor;
    SCREEN_X =  RESOLUTION.x/(zoom_factor * 2);
    SCREEN_Y =  RESOLUTION.y/(zoom_factor * 2);
}

float Renderer::GetCameraZoom(){
    return zoom_factor;
}

void Renderer::AddImageRequest(const ImageRequest& req){
    int idx = write_index.load();
    render_frames[idx].image_requests.push_back(req);
}

void Renderer::AddUIRequest(const UIRequest& req){
    int idx = write_index.load();
    render_frames[idx].UI_requests.push_back(req);
}

void Renderer::AddTextRequest(const TextRequest& req){
    int idx = write_index.load();
    render_frames[idx].text_requests.push_back(req);
}

void Renderer::AddPixelRequest(const PixelRequest& req){
    int idx = write_index.load();
    render_frames[idx].pixel_requests.push_back(req);
}

void Renderer::rendererClear(){
        SDL_SetRenderDrawColor(Renderer::sdl_renderer, Renderer::clear_color.r, 
            Renderer::clear_color.g, Renderer::clear_color.b, 0);
    SDL_RenderClear(Renderer::sdl_renderer);
}

SDL_Renderer* Renderer::GetRenderer(){
    return sdl_renderer;
}
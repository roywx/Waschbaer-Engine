#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>
#include <queue>
#include <atomic>

#include "glm/glm.hpp"
#include "Actor.h"
#include "SceneDB.h"
#include "SDL2/SDL.h"
#include "SDL_image/SDL_image.h"
#include "ImageDB.h"
#include "EngineUtils.h"
#include "SDL_ttf/SDL_ttf.h"

class Renderer
{
public:    
    struct TextRequest{
        float x, y;
        TTF_Font* font;
        SDL_Color color;
        std::string text;
        TextRequest(std::string pt, float px, float py, TTF_Font* pf, SDL_Color pc)
            : x(px), y(py), font(pf), color(pc), text(pt)  {}
    };

    struct UIRequest{
        uint8_t r, g, b, a;
        float x, y;
        int sorting_order;
        SDL_Texture* texture;

        UIRequest(SDL_Texture* pt, float px, float py, float ps, uint8_t pr, uint8_t pg, uint8_t pb, uint8_t pa) : 
            r(pr), g(pg), b(pb), a(pa), x(px), y(py), sorting_order(ps), texture(pt){}
    };

     struct ImageRequest{
        uint8_t r, g, b, a;
        float x, y;   
        int rot_deg = 0;
        int sorting_order = 0;
        float scale_x = 1.0f;
        float scale_y = 1.0f;
        float pivot_x = 0.5f;
        float pivot_y = 0.5f;
        SDL_Texture* texture;
        float cached_w, cached_h;   

        ImageRequest(SDL_Texture* pt, float px, float py, int pso, uint8_t pr, uint8_t pg, uint8_t pb, uint8_t pa, float pw, float ph) : 
            r(pr), g(pg), b(pb), a(pa), x(px), y(py), sorting_order(pso), texture(pt), cached_w(pw), cached_h(ph) {}
        ImageRequest(SDL_Texture* pt, float px, float py, float prt, float psx, float psy, float ppx, float ppy, float pso, 
                uint8_t pr, uint8_t pg, uint8_t pb, uint8_t pa, float pw, float ph) :
            r(pr), g(pg), b(pb), a(pa), x(px), y(py), rot_deg(prt), sorting_order(pso),
            scale_x(psx), scale_y(psy), pivot_x(ppx), pivot_y(ppy), texture(pt), cached_w(pw), cached_h(ph){}
    };

    struct PixelRequest{
        uint8_t r, g, b, a;
        int x, y;
        PixelRequest(float px, float py, uint8_t pr, uint8_t pg, uint8_t pb, uint8_t pa)
            : r(pr), g(pg), b(pb), a(pa), x(px), y(py)  {}
    };

    struct RenderFrame {
        std::vector<ImageRequest> image_requests;
        std::vector<UIRequest>    UI_requests;
        std::vector<TextRequest>  text_requests;
        std::vector<PixelRequest> pixel_requests;
    };

    static inline std::atomic<int> write_index{0};

    static void updateRender(const glm::vec2& playerPos, const SceneDB* sceneDB); 
    static void updateRender(const glm::vec2& playerPos, const SceneDB* sceneDB, const glm::vec2& dir); 
    static void init();

    static void renderAllTexts(int frame_idx);
    static void renderAllImages(int frame_idx);
    static void renderAllUI(int frame_idx);
    static void renderAllPixels(int frame_idx);
    static void rendererClear();

    static void AddImageRequest(const ImageRequest& req);
    static void AddUIRequest(const UIRequest& req);
    static void AddTextRequest(const TextRequest& req);
    static void AddPixelRequest(const PixelRequest& req);

    static void SetCameraPosition(float x, float y);
    static float GetCameraPositionX();
    static float GetCameraPositionY();
    static void SetCameraZoom(float zoom);
    static float GetCameraZoom();

    static SDL_Renderer* GetRenderer();
private:
    static inline std::string game_title;

    static inline SDL_Color clear_color = {255, 255, 255, 0};
    static inline SDL_Window* window;
    static inline SDL_Renderer* sdl_renderer;
    
    static inline glm::ivec2 RESOLUTION = glm::ivec2(640, 360);;
    static inline float PIXEL_DIST = 100.0f;

    static inline std::array<RenderFrame, 2> render_frames;

    // Camera Controls
    static inline glm::vec2 cam_pos = glm::vec2(0.0f, 0.0f);
    static inline glm::vec2 cam_offset = glm::vec2(0.0f, 0.0f);;
    static inline float zoom_factor = 1.0f;
    // Cached values
    static inline float inv_zoom = 1.0f;
    static inline float SCREEN_X = 320.0f;
    static inline float SCREEN_Y =  180.0f;
};

#endif
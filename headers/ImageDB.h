#ifndef IMAGE_DB_H
#define IMAGE_DB_H

#include <queue>
#include <string>
#include <unordered_map>
#include <memory>

#include "SDL2/SDL.h"
#include "SDL_image/SDL_image.h"
#include "Actor.h"
#include "glm/glm.hpp"

class ImageDB{
private:
    static inline std::unordered_map<std::string, SDL_Texture*> images;
    static SDL_Texture* load_img(std::string img_name);
    static SDL_Texture* GetImage(const std::string& image_name);
public:
    static void PreloadAll();
    static void DrawUI(std::string img_name, float x, float y);
    static void DrawUIEx(std::string img_name, float x, float y, float r, float g, float b, float a, float sorting_order);
    static void Draw(std::string img_name, float x, float y);
    static void DrawEx(std::string img_name, float x, float y, float rot_deg, float scale_x, float scale_y, float pivot_x, 
        float pivot_y, float r, float g, float b, float a, float sorting_order);
    static void DrawPixel(float x, float y, float r, float g, float b, float a);
    static void CreateDefaultParticleTextureWithName(const std::string name);
};

#endif
#ifndef TEXT_DB
#define TEXT_DB

#include <string>
#include <queue>
#include <unordered_map>

#include "SDL2/SDL.h"
#include "SDL_ttf/SDL_ttf.h"
#include "glm/glm.hpp"

class TextDB{
private:
    static inline std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> fonts;
public:
    static void DrawText(const std::string& text, float x, float y, std::string font_name, int font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    static void init();
    static void shutdown();
    
};

#endif
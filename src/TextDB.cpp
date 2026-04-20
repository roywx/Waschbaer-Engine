#include "TextDB.h"

#include "rapidjson/document.h"
#include "EngineUtils.h"
#include "Renderer.h"
#include "Helper.h"

#include <filesystem>
#include <iostream>

void TextDB::init(){
    TTF_Init();    
}

void TextDB::DrawText(const std::string& text, float x, float y, std::string font_name, int font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    if(!fonts.count(font_name) || !fonts.at(font_name).count(font_size)){
        std::string file_path = ("resources/fonts/" + font_name + ".ttf");
        if(!std::filesystem::exists(file_path)){
            std::cout << "error: font " << font_name << " missing"; 
            exit(0);
        }
       fonts[font_name][font_size] = TTF_OpenFont(file_path.c_str(), font_size);
    }
    TTF_Font* f = fonts.at(font_name).at(font_size);
    Renderer::AddTextRequest(Renderer::TextRequest(text, x, y, f, SDL_Color{r,g,b,a}));
}

void TextDB::shutdown(){
    for(auto& [font_name, ttf_map] : fonts){
       for(auto& [font_size, ttf_font] : ttf_map){
           TTF_CloseFont(ttf_font);
       }
    }
}


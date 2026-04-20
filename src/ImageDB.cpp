#include "ImageDB.h"

#include "rapidjson/document.h"
#include "EngineUtils.h"
#include "Renderer.h"
#include "Helper.h"
#include "ThreadPool.h"

#include <filesystem>
#include <iostream>

void ImageDB::PreloadAll(){
    static const std::string FILE_PATH = "resources/images/";
    if(!std::filesystem::exists(FILE_PATH)) return;

    std::vector<std::filesystem::path> paths;
    for(const auto& entry : std::filesystem::directory_iterator(FILE_PATH))
        if(entry.path().extension() == ".png")
            paths.push_back(entry.path());

    std::vector<SDL_Surface*> surfaces(paths.size(), nullptr);

    ThreadPool::ParallelFor((int)paths.size(), [&](int start, int end){
        for(int i = start; i < end; i++){
            surfaces[i] = IMG_Load(paths[i].string().c_str());
        }
    });

    for(size_t i = 0; i < paths.size(); i++){
        if(!surfaces[i]) continue;
        std::string name = paths[i].stem().string();
        images[name] = SDL_CreateTextureFromSurface(Renderer::GetRenderer(), surfaces[i]);
        SDL_FreeSurface(surfaces[i]);
    }
}

SDL_Texture* ImageDB::load_img(std::string img_name){
    static std::string FILE_PATH = "resources/images/";
    static std::string EXTENSION = ".png";
    std::string file_path = FILE_PATH + img_name + EXTENSION;
    if(!std::filesystem::exists(file_path)){
        std::cout << "error: missing image " << img_name;
        exit(0);
    }
    return IMG_LoadTexture(Renderer::GetRenderer(), file_path.c_str());
}


SDL_Texture* ImageDB::GetImage(const std::string& image_name){
    if(images.count(image_name)){
        return images[image_name];
    }else{
        images[image_name] = load_img(image_name);
        return images[image_name];
    }
}

void ImageDB::DrawUI(std::string img_name, float x, float y){
    SDL_Texture* texture = GetImage(img_name);
    Renderer::AddUIRequest(Renderer::UIRequest(texture, x, y, 0, 255, 255, 255, 255));
}

void ImageDB::DrawUIEx(std::string img_name, float x, float y, float r, float g, float b, float a, float sorting_order){
    SDL_Texture* texture = GetImage(img_name);
    Renderer::AddUIRequest(Renderer::UIRequest(texture, x, y, sorting_order, r, g, b, a));
}

void ImageDB::Draw(std::string img_name, float x, float y){
    SDL_Texture* texture = GetImage(img_name);
    float w, h;
    Helper::SDL_QueryTexture(texture, &w, &h);
    Renderer::AddImageRequest(Renderer::ImageRequest(texture, x, y, 0, 255, 255, 255, 255, w, h));
}

void ImageDB::DrawEx(std::string img_name, float x, float y, float rot_deg, float scale_x, float scale_y, 
    float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order){
    SDL_Texture* texture = GetImage(img_name);
    float w, h;
    Helper::SDL_QueryTexture(texture, &w, &h);
    Renderer::AddImageRequest(Renderer::ImageRequest(texture, x, y,rot_deg, scale_x, scale_y, pivot_x, pivot_y, sorting_order, r, g, b, a, w, h));
}

void ImageDB::DrawPixel(float x, float y, float r, float g, float b, float a){
    Renderer::AddPixelRequest(Renderer::PixelRequest(x, y, r, g, b, a));
}

void ImageDB::CreateDefaultParticleTextureWithName(const std::string name){
    if(images.count(name)) return;

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);

    uint32 white_color = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
    SDL_FillRect(surface, NULL, white_color);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer::GetRenderer(), surface);
    SDL_FreeSurface(surface);

    images[name] = texture;
}
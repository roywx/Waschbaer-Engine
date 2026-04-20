#ifndef INPUT_H
#define INPUT_H

#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "SDL2/SDL.h"
#include <string>

enum INPUT_STATE {INPUT_STATE_UP, INPUT_STATE_JUST_UP, INPUT_STATE_DOWN, INPUT_STATE_JUST_DOWN};

class Input{
public:
    static void Init();
    static void ProcessEvent(const SDL_Event& e);
    static void LateUpdate();

    static bool GetKey(const SDL_Scancode& keycode);
    static bool GetKeyDown(const SDL_Scancode& keycode);
    static bool GetKeyUp(const SDL_Scancode& keycode);
   
    static bool GetKey(const std::string& keycode);
    static bool GetKeyDown(const std::string& keycode);
    static bool GetKeyUp(const std::string& keycode);

    static bool GetMouseButton(int button);
    static bool GetMouseButtonDown(int button);
    static bool GetMouseButtonUp(int button);
    
    static float GetMouseScrollData();
    static glm::vec2 GetMousePosition();

    static void ShowCursor();
    static void HideCursor();

private:
    static inline std::vector<SDL_Scancode> just_down_scancodes;
    static inline std::vector<SDL_Scancode> just_up_scancodes;
    static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;

    static inline glm::vec2 mouse_position;
    static inline float mouse_scroll_this_frame = 0;
    static inline std::unordered_map<int, INPUT_STATE> mouse_button_states;
    static inline std::vector<int> just_down_buttons;
    static inline std::vector<int> just_up_buttons;
};


#endif
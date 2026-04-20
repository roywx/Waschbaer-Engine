#include "Input.h"

const std::unordered_map<std::string, SDL_Scancode> __keycode_to_scancode = {
	// Directional (arrow) Keys
	{"up", SDL_SCANCODE_UP},
	{"down", SDL_SCANCODE_DOWN},
	{"right", SDL_SCANCODE_RIGHT},
	{"left", SDL_SCANCODE_LEFT},

	// Misc Keys
	{"escape", SDL_SCANCODE_ESCAPE},

	// Modifier Keys
	{"lshift", SDL_SCANCODE_LSHIFT},
	{"rshift", SDL_SCANCODE_RSHIFT},
	{"lctrl", SDL_SCANCODE_LCTRL},
	{"rctrl", SDL_SCANCODE_RCTRL},
	{"lalt", SDL_SCANCODE_LALT},
	{"ralt", SDL_SCANCODE_RALT},

	// Editing Keys
	{"tab", SDL_SCANCODE_TAB},
	{"return", SDL_SCANCODE_RETURN},
	{"enter", SDL_SCANCODE_RETURN},
	{"backspace", SDL_SCANCODE_BACKSPACE},
	{"delete", SDL_SCANCODE_DELETE},
	{"insert", SDL_SCANCODE_INSERT},

	// Character Keys
	{"space", SDL_SCANCODE_SPACE},
	{"a", SDL_SCANCODE_A},
	{"b", SDL_SCANCODE_B},
	{"c", SDL_SCANCODE_C},
	{"d", SDL_SCANCODE_D},
	{"e", SDL_SCANCODE_E},
	{"f", SDL_SCANCODE_F},
	{"g", SDL_SCANCODE_G},
	{"h", SDL_SCANCODE_H},
	{"i", SDL_SCANCODE_I},
	{"j", SDL_SCANCODE_J},
	{"k", SDL_SCANCODE_K},
	{"l", SDL_SCANCODE_L},
	{"m", SDL_SCANCODE_M},
	{"n", SDL_SCANCODE_N},
	{"o", SDL_SCANCODE_O},
	{"p", SDL_SCANCODE_P},
	{"q", SDL_SCANCODE_Q},
	{"r", SDL_SCANCODE_R},
	{"s", SDL_SCANCODE_S},
	{"t", SDL_SCANCODE_T},
	{"u", SDL_SCANCODE_U},
	{"v", SDL_SCANCODE_V},
	{"w", SDL_SCANCODE_W},
	{"x", SDL_SCANCODE_X},
	{"y", SDL_SCANCODE_Y},
	{"z", SDL_SCANCODE_Z},
	{"0", SDL_SCANCODE_0},
	{"1", SDL_SCANCODE_1},
	{"2", SDL_SCANCODE_2},
	{"3", SDL_SCANCODE_3},
	{"4", SDL_SCANCODE_4},
	{"5", SDL_SCANCODE_5},
	{"6", SDL_SCANCODE_6},
	{"7", SDL_SCANCODE_7},
	{"8", SDL_SCANCODE_8},
	{"9", SDL_SCANCODE_9},
	{"/", SDL_SCANCODE_SLASH},
	{";", SDL_SCANCODE_SEMICOLON},
	{"=", SDL_SCANCODE_EQUALS},
	{"-", SDL_SCANCODE_MINUS},
	{".", SDL_SCANCODE_PERIOD},
	{",", SDL_SCANCODE_COMMA},
	{"[", SDL_SCANCODE_LEFTBRACKET},
	{"]", SDL_SCANCODE_RIGHTBRACKET},
	{"\\", SDL_SCANCODE_BACKSLASH},
	{"'", SDL_SCANCODE_APOSTROPHE}
};

void Input::Init(){
    for(int code = SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; code++){
        keyboard_states[static_cast<SDL_Scancode>(code)] = INPUT_STATE_UP;
    }
    just_down_scancodes.reserve(SDL_NUM_SCANCODES);
    just_up_scancodes.reserve(SDL_NUM_SCANCODES);
}

void Input::ProcessEvent(const SDL_Event& e){
    if(e.type == SDL_KEYDOWN){
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_DOWN;
        just_down_scancodes.push_back(e.key.keysym.scancode);
    }else if(e.type == SDL_KEYUP){
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_UP;
        just_up_scancodes.push_back(e.key.keysym.scancode);
    }else if(e.type == SDL_MOUSEBUTTONDOWN){
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_DOWN;
		just_down_buttons.push_back(e.button.button);
    }else if(e.type == SDL_MOUSEBUTTONUP){
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_UP;
		just_up_buttons.push_back(e.button.button);
    }else if(e.type == SDL_MOUSEMOTION){
        mouse_position = {e.motion.x, e.motion.y};
    }else if(e.type == SDL_MOUSEWHEEL){
        mouse_scroll_this_frame = e.wheel.preciseY;
    }
}

void Input::LateUpdate(){
    for(const SDL_Scancode& code : just_down_scancodes){
        keyboard_states[code] = INPUT_STATE_DOWN;
    }
    just_down_scancodes.clear();

    for(const SDL_Scancode& code : just_up_scancodes){
        keyboard_states[code] = INPUT_STATE_UP;
    }
    just_up_scancodes.clear();

    for(const int& button_event : just_down_buttons){
        mouse_button_states[button_event] = INPUT_STATE_DOWN;
    }
    just_down_buttons.clear();
    for(const int& button_event : just_up_buttons){
        mouse_button_states[button_event] = INPUT_STATE_UP;
    }
    just_up_buttons.clear();
    mouse_scroll_this_frame = 0;
}

bool Input::GetKey(const SDL_Scancode& keycode){
    return (keyboard_states[keycode] == INPUT_STATE_JUST_DOWN || keyboard_states[keycode]  == INPUT_STATE_DOWN);
}
bool Input::GetKeyDown(const SDL_Scancode& keycode){
    return keyboard_states[keycode] == INPUT_STATE_JUST_DOWN;
}
bool Input::GetKeyUp(const SDL_Scancode& keycode){
    return keyboard_states[keycode] == INPUT_STATE_JUST_UP;
}

bool Input::GetKey(const std::string& keycode){
    if(!__keycode_to_scancode.count(keycode)) return false;
    SDL_Scancode sdl_key = __keycode_to_scancode.at(keycode);
    return (keyboard_states[sdl_key] == INPUT_STATE_JUST_DOWN || keyboard_states[sdl_key]  == INPUT_STATE_DOWN);
}
bool Input::GetKeyDown(const std::string& keycode){
    if(!__keycode_to_scancode.count(keycode)) return false;
    SDL_Scancode sdl_key = __keycode_to_scancode.at(keycode);
    return keyboard_states[sdl_key] == INPUT_STATE_JUST_DOWN;
}
bool Input::GetKeyUp(const std::string& keycode){
    if(!__keycode_to_scancode.count(keycode)) return false;
    SDL_Scancode sdl_key = __keycode_to_scancode.at(keycode);
    return keyboard_states[sdl_key] == INPUT_STATE_JUST_UP;
}

bool Input::GetMouseButton(int button){
    return (mouse_button_states[button] == INPUT_STATE_JUST_DOWN || mouse_button_states[button]  == INPUT_STATE_DOWN);
};
bool Input::GetMouseButtonDown(int button){
    return mouse_button_states[button] == INPUT_STATE_JUST_DOWN;
};
bool Input::GetMouseButtonUp(int button){
    return mouse_button_states[button] == INPUT_STATE_JUST_UP;
};

float Input::GetMouseScrollData(){
    return mouse_scroll_this_frame;
};
glm::vec2 Input::GetMousePosition(){
    return mouse_position;
};

void Input::ShowCursor(){
    SDL_ShowCursor(SDL_ENABLE);
}

void Input::HideCursor(){
    SDL_ShowCursor(SDL_DISABLE);
}
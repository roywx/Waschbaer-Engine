#ifndef INPUTHELPER_H
#define INPUTHELPER_H

#include <unordered_map>
#include <queue>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <random>

#include "SDL_image/SDL_image.h"
#include "SDL2/SDL.h"

enum InputStatus { NOT_INITIALIZED, INPUT_FILE_MISSING, INPUT_FILE_PRESENT };

class Helper {
public:
	inline static const char* USER_INPUT_FILENAME = "sdl_user_input.txt";

	inline static std::string frame_directory_relative_path = "frames";

	/* The frame_number advances with every call to Helper::SDL_RenderPresent() */
	static inline int frame_number = 0;
	static inline Uint32 current_frame_start_timestamp = 0;
	static int GetFrameNumber() { return frame_number; }

	static SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
	{
		return ::SDL_CreateWindow(title, x, y, w, h, flags);
	}

	static SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, int index, Uint32 flags)
	{
		SDL_Renderer* renderer = ::SDL_CreateRenderer(window, index, flags);

		if (renderer == nullptr)
			std::cerr << "Failed to create renderer : " << SDL_GetError() << std::endl;

		return renderer;
	}

	/* Wrapper that will inject input events into the SDL Event Queue if a user input file is found */
	/* This is what enables playback of inputs and game session replay. */
	static int SDL_PollEvent(SDL_Event* e)
	{
		SDL_ConsiderInputFile();
		return ::SDL_PollEvent(e);
	}

	/* Wrapper that renders to screen while also persisting to a .BMP file */
	static void SDL_RenderPresent(SDL_Renderer* renderer)
	{
		if (renderer == nullptr)
		{
			std::cout << "ERROR : The renderer pointer passed to Helper::SDL_RenderPresent() is a nullptr." << std::endl;
			exit(0);
		}

		if (input_status == NOT_INITIALIZED)
		{
			std::cout << "ERROR : Please do not attempt to render before entering the game loop (IE, begun calling Helper::SDL_PollEvent()." << std::endl;
			exit(0);
		}

		::SDL_RenderPresent(renderer); 
		SDL_Delay();
		frame_number++;
	}

	static void SDL_RenderCopyEx(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect* srcrect, const SDL_FRect* dstrect, const float angle, const SDL_FPoint* center, const SDL_RendererFlip flip)
	{
		SDL_Rect* srcrect_i_ptr = nullptr;
		SDL_Rect* dstrect_i_ptr = nullptr;
		SDL_Point* center_i_ptr = nullptr;

		SDL_Rect srcrect_i, dstrect_i;

		SDL_Point center_i;

		if (srcrect != nullptr)
		{
			srcrect_i = { static_cast<int>(srcrect->x), static_cast<int>(srcrect->y), static_cast<int>(srcrect->w), static_cast<int>(srcrect->h) };
			srcrect_i_ptr = &srcrect_i;
		}
		
		if (dstrect != nullptr)
		{
			dstrect_i = { static_cast<int>(dstrect->x), static_cast<int>(dstrect->y), static_cast<int>(dstrect->w), static_cast<int>(dstrect->h) };
			dstrect_i_ptr = &dstrect_i;
		}

		if (center != nullptr)
		{
			center_i = { static_cast<int>(center->x), static_cast<int>(center->y) };
			center_i_ptr = &center_i;
		}

		/* Perform the render like normal. */
		::SDL_RenderCopyEx(renderer, texture, srcrect_i_ptr, dstrect_i_ptr, angle, center_i_ptr, flip);
	}

	static void SDL_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect * srcrect, const SDL_FRect * dstrect)
	{
		SDL_Rect* srcrect_i_ptr = nullptr;
		SDL_Rect* dstrect_i_ptr = nullptr;

		SDL_Rect srcrect_i, dstrect_i;

		if (srcrect != nullptr)
		{
			srcrect_i = { static_cast<int>(srcrect->x), static_cast<int>(srcrect->y), static_cast<int>(srcrect->w), static_cast<int>(srcrect->h) };
			srcrect_i_ptr = &srcrect_i;
		}

		if (dstrect != nullptr)
		{
			dstrect_i = { static_cast<int>(dstrect->x), static_cast<int>(dstrect->y), static_cast<int>(dstrect->w), static_cast<int>(dstrect->h) };
			dstrect_i_ptr = &dstrect_i;
		}

		::SDL_RenderCopy(renderer, texture, srcrect_i_ptr, dstrect_i_ptr);
	}

	static void SDL_QueryTexture(SDL_Texture* texture, float* w, float* h)
	{
		if (texture == nullptr)
			return;

		int w_i, h_i;
		::SDL_QueryTexture(texture, NULL, NULL, &w_i, &h_i);

		if (w != nullptr)
			*w = static_cast<float>(w_i);

		if (h != nullptr)
			*h = static_cast<float>(h_i);
	}

private:
	static inline std::unordered_map<int, std::queue<SDL_Event>> frame_to_user_input;
	static inline InputStatus input_status = NOT_INITIALIZED;
	static inline std::ofstream recording_file;

	/* Do not use SDL_GetKeyboardState(), as it will not observe the input file. */
	static void SDL_ConsiderInputFile()
	{
		/* Lazy Initialize */
		if (input_status == NOT_INITIALIZED)
		{
			LoadSDLEventsFromInputFile();
		}

		if (input_status == INPUT_FILE_PRESENT)
		{
			if (frame_to_user_input.find(frame_number) != frame_to_user_input.end())
			{
				while (!frame_to_user_input[frame_number].empty())
				{
					SDL_PushEvent(&(frame_to_user_input[frame_number].front()));
					frame_to_user_input[frame_number].pop();
				}
			}
		}
	}

	/* The engine will aim for 60fps (16ms per frame) during a normal play session. */
	static void SDL_Delay() {

		Uint32 current_frame_end_timestamp = SDL_GetTicks();  // Record end time of the frame
		Uint32 current_frame_duration_milliseconds = current_frame_end_timestamp - current_frame_start_timestamp;
		Uint32 desired_frame_duration_milliseconds = 16;

		int delay_ticks = std::max(static_cast<int>(desired_frame_duration_milliseconds) - static_cast<int>(current_frame_duration_milliseconds), 1);

		::SDL_Delay(delay_ticks);
		
		current_frame_start_timestamp = SDL_GetTicks();  // Record start time of the frame
	}

	static void LoadSDLEventsFromInputFile()
	{
		if (!std::filesystem::exists(USER_INPUT_FILENAME))
		{
			input_status = INPUT_FILE_MISSING;
			return;
		}

		std::ifstream infile(USER_INPUT_FILENAME);
		std::string line;

		while (std::getline(infile, line)) {
			std::istringstream iss(line);
			std::string eventStr, frameStr;

			std::getline(iss, frameStr, ';');
			int frameNumber = std::stoi(frameStr);

			if (frame_to_user_input.find(frameNumber) == frame_to_user_input.end())
				frame_to_user_input[frameNumber] = std::queue<SDL_Event>();

			std::queue<SDL_Event> & new_queue = frame_to_user_input[frameNumber];

			while (std::getline(iss, eventStr, ';')) {

				/* Identify event type */
				std::istringstream eventStream(eventStr);
				std::string event_type_string;
				std::getline(eventStream, event_type_string, ',');

				// Remove any '\r' carriage returns we might find
				// (may be needed for stoi to work on osx / linux when sdl_user_input.txt has been authored on windows)
				event_type_string.erase(std::remove(event_type_string.begin(), event_type_string.end(), '\r'), event_type_string.end());

				if (event_type_string == "")
					continue;

				Uint32 event_type = std::stoi(event_type_string);
				SDL_Event fabricated_sdl_event;
				fabricated_sdl_event.type = event_type;

				/* Handle unpacking for different event types */

				if (event_type == SDL_KEYUP || event_type == SDL_KEYDOWN)
				{
					std::string keycode;
					std::getline(eventStream, keycode, ',');
					if (keycode == "")
						continue;

					fabricated_sdl_event.key.keysym.scancode = static_cast<SDL_Scancode>(std::stoi(keycode));
				}
				else if (event_type == SDL_MOUSEMOTION)
				{
					std::string x_str;
					std::getline(eventStream, x_str, ',');
					std::string y_str;
					std::getline(eventStream, y_str, ',');

					if (x_str == "" || y_str == "")
						continue;

					fabricated_sdl_event.motion.x = static_cast<Sint32>(std::stoi(x_str));
					fabricated_sdl_event.motion.y = static_cast<Sint32>(std::stoi(y_str));
				}
				else if (event_type == SDL_MOUSEBUTTONDOWN || event_type == SDL_MOUSEBUTTONUP)
				{
					std::string mouse_button_index_str;
					std::getline(eventStream, mouse_button_index_str, ',');

					if (mouse_button_index_str == "")
						continue;

					fabricated_sdl_event.button.button = static_cast<Uint8>(std::stoi(mouse_button_index_str));
				}
				else if (event_type == SDL_MOUSEWHEEL)
				{
					std::string mouse_wheel_movement_str;
					std::getline(eventStream, mouse_wheel_movement_str, ','); 

					if (mouse_wheel_movement_str == "")
						continue;

					fabricated_sdl_event.wheel.preciseY = std::stof(mouse_wheel_movement_str);
				}

				new_queue.push(fabricated_sdl_event);
			}
		}

		input_status = INPUT_FILE_PRESENT;
	}
};

class RandomEngine
{
	std::default_random_engine engine;
	std::uniform_real_distribution<float> distribution;

public:
	RandomEngine(float min, float max, int seed)
	{
		Configure(min, max, seed);
	}

	void Configure(float min, float max, int seed)
	{
		engine = std::default_random_engine(seed);
		distribution = std::uniform_real_distribution<float>(min, max);
	}

	RandomEngine() {}

	/* Get a random number in the specified range. */
	inline float Sample()
	{
		return distribution(engine);
	}
};

#endif
//#include <cstdio>
#include <string>

#include "SDLEssentials.h"
#include "Vector.h"


const char* TITLE{ "LinkageSim" };
int screen_width{ 640 };
int screen_height{ 480 };
const Uint64 TICKS_PER_SECOND{ SDL_GetPerformanceFrequency() };
const Uint64 BLINKING_TIME{ static_cast<Uint64>(TICKS_PER_SECOND * .5) };

enum KeysType
{
	PLACEHOLDER,
	MAX_KEYS
};

enum MouseButtonsType
{
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,
	MAX_MOUSEBUTTONS
};

struct ButtonStates
{
	bool down{ false };
	bool held{ false };
	bool up{ false };
};

void resetButtonStates(ButtonStates* array, int length)
{
	for(int i{ 0 }; i < length; ++i)
	{
		array[i].down = false;
		array[i].up = false;
	}
}

int main(int argc, char** argv)
{
	SDL_Window* window{ nullptr };
	SDL_Renderer* renderer{ nullptr };

	if(!init(window, TITLE, screen_width, screen_height, renderer))
	{
		std::printf("Couldn't initialize!\n");
	}
	else
	{
		//Measuring fps
		double fps{ 0.0 };
		double fps_sum{ 0.0 };
		double fps_avg{ 0.0 };
		int fps_sum_i{ 1 };
		int start_time{ 0 };
		int end_time{ 0 };


		//Text:
		//string, path, size, color, texture
		std::string text{"Essa"};

		std::string font_path{ "fonts/Inconsolata-Regular.ttf" };

		int font_size{ 20 };

		SDL_Color text_color{ 0xff, 0xff, 0xff, 0xff };
		SDL_Color background_color{ 0x0, 0x0, 0x0, 0xff };

		Texture text_texture;



		ButtonStates keyboard[MAX_KEYS];
		ButtonStates mouse[MAX_MOUSEBUTTONS];
		int mouse_x{ 0 }, mouse_y{ 0 };




		bool keysPressed{ false };

		bool quit{ false };
		SDL_Event e{};

		SDL_StartTextInput();
		while(!quit)
		{
			while(SDL_PollEvent(&e))
			{
				if(SDL_QUIT == e.type)
				{
					quit = true;
				}

				if(SDL_KEYDOWN == e.type && e.key.repeat == 0)
				{


					keysPressed = true;
				}

				if(SDL_KEYUP == e.type && e.key.repeat == 0)
				{

				}

				if(SDL_MOUSEMOTION == e.type)
				{
					SDL_GetMouseState(&mouse_x, &mouse_y);
				}

				if(SDL_MOUSEBUTTONDOWN == e.type)
				{
					if(SDL_BUTTON_LEFT == e.button.button)
					{
						//std::printf("Left mouse button down.\n");
						mouse[MOUSE_LEFT].down = true;
						mouse[MOUSE_LEFT].held = true;
					}
					if(SDL_BUTTON_RIGHT == e.button.button)
					{
						//std::printf("Right mouse button down.\n");
						mouse[MOUSE_RIGHT].down = true;
						mouse[MOUSE_RIGHT].held = true;
					}
					if(SDL_BUTTON_MIDDLE == e.button.button)
					{
						//std::printf("Middle mouse button down.\n");
						mouse[MOUSE_MIDDLE].down = true;
						mouse[MOUSE_MIDDLE].held = true;
					}
				}

				if(SDL_MOUSEBUTTONUP == e.type)
				{
					if(SDL_BUTTON_LEFT == e.button.button)
					{
						//std::printf("Left mouse button up.\n");
						mouse[MOUSE_LEFT].up = true;
						mouse[MOUSE_LEFT].held = false;
					}
					if(SDL_BUTTON_RIGHT == e.button.button)
					{
						//std::printf("Right mouse button up.\n");
						mouse[MOUSE_RIGHT].up = true;
						mouse[MOUSE_RIGHT].held = false;
					}
					if(SDL_BUTTON_MIDDLE == e.button.button)
					{
						//std::printf("Middle mouse button up.\n");
						mouse[MOUSE_MIDDLE].up = true;
						mouse[MOUSE_MIDDLE].held = false;
					}
				}

				if(e.type == SDL_MOUSEWHEEL)
				{
					;
				}
			}
			start_time = SDL_GetPerformanceCounter();

			SDL_GetRendererOutputSize(renderer, &screen_width, &screen_height);

			TTF_Font* text_font{ TTF_OpenFont(font_path.c_str(), font_size) };

			SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
			SDL_RenderClear(renderer);
			 
			text_texture.loadFromRenderedText(renderer, text_font, text, text_color, background_color);

			text_texture.render(renderer, 0, 0, NULL);


			fps_sum += fps;
			++fps_sum_i;
			if(fps_sum_i > 100)
			{
				 fps_avg = fps_sum / 100.0;
				 text = std::to_string(fps_avg);
				 fps_sum = 0;
				 fps_avg = 0;
				 fps_sum_i = 1;
			}
			
			SDL_RenderPresent(renderer);

			resetButtonStates(mouse, MAX_MOUSEBUTTONS);
			resetButtonStates(keyboard, MAX_KEYS);
			keysPressed = false;

			text_texture.free();
			TTF_CloseFont(text_font);

			end_time = SDL_GetPerformanceCounter();

			int delta_time{ static_cast<int>(end_time - start_time) };
			fps = TICKS_PER_SECOND / double(delta_time);
		}

		close(window, renderer);

		return 0;
	}
}
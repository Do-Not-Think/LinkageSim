//#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

#include "LinkageSim.h"
#include "MathConsts.h"
#include "SDLEssentials.h"
#include "Vector.h"

const char* TITLE{ "LinkageSim" };
int screen_width{ 640 };
int screen_height{ 480 };
const Uint64 TICKS_PER_SECOND{ LinkageSim::TICKS_PER_SECOND };
const int FRAMERATE{ LinkageSim::FRAMERATE };
const int FRAME{ LinkageSim::FRAME }; //Ticks per frame
const int AVG_FPS_FRAME_COUNT{ 120 };

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
		int delta_time{ 0 };

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



		
		/*   LinkageSim   */
		
		//Using delta_time
		bool using_delta_time{ false };

		//Objects
		std::vector<Arm> arms{};
		
		Arm motor0{};
		motor0.pos = Vector_2d{ 100, 400 };
		motor0.length = 100;
		motor0.angle = -PI / 2.0;
		motor0.previous_angle = motor0.angle;
		int motor0_period = 1 * TICKS_PER_SECOND;
		double rod0_length{ 300 };
		motor0.connected_arms.push_back(Index_RodLength{ 1, rod0_length });
		arms.push_back(motor0);

		Arm arm0{};
		arm0.pos = Vector_2d{ 350, 400 };
		arm0.angle = -PI / 2.0;
		arm0.previous_angle = arm0.angle;
		arm0.length = 200;
		arms.push_back(arm0);



		bool keysPressed{ false };

		bool quit{ false };
		SDL_Event e{};
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
			
			updateMotor(arms, 0, motor0_period);
			
			Vector_2d pos1{};
			Vector_2d pos2{};
			
			//motor0 arm
			SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);
			pos1 = arms[0].pos;
			pos2 = arms[0].pos + polarToVector(arms[0].angle, arms[0].length);
			SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			//rod connecting motor0 and arm0 
			pos1 = pos2;
			pos2 = arms[1].pos + polarToVector(arms[1].angle, arms[1].length);
			SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);
			//arm0 arm
			pos1 = pos2;
			pos2 = arms[1].pos;
			SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);


			Vector_2d connecting_rod;
			connecting_rod.x = arms[1].pos.x + std::cos(arms[1].angle) * arms[1].length;
			connecting_rod.y = arms[1].pos.y + std::sin(arms[1].angle) * arms[1].length;
			connecting_rod.x -= arms[0].pos.x + std::cos(arms[0].angle) * arms[0].length;
			connecting_rod.y -= arms[0].pos.y + std::sin(arms[0].angle) * arms[0].length;

			connecting_rod *= 2;

			Vector_2d final_pos;
			final_pos.x = connecting_rod.x + arms[0].pos.x + std::cos(arms[0].angle) * arms[0].length;
			final_pos.y = connecting_rod.y + arms[0].pos.y + std::sin(arms[0].angle) * arms[0].length;


			SDL_RenderDrawLine(renderer, arms[0].pos.x + std::cos(arms[0].angle) * arms[0].length, arms[0].pos.y + std::sin(arms[0].angle) * arms[0].length, final_pos.x, final_pos.y);
						
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);		

			SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
			
			text_texture.loadFromRenderedText(renderer, text_font, text, text_color, background_color);

			text_texture.render(renderer, 0, 0, NULL);

			fps_sum += fps;
			++fps_sum_i;
			if(fps_sum_i > AVG_FPS_FRAME_COUNT)
			{
				 fps_avg = fps_sum / AVG_FPS_FRAME_COUNT;
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
			delta_time = static_cast<int>(end_time - start_time);
			
			fps = TICKS_PER_SECOND / double(delta_time);

			if(!using_delta_time)
			{
				//Busy loop, until frame takes as much time as it should
				while(delta_time < FRAME)
				{
					end_time = SDL_GetPerformanceCounter();
					delta_time = static_cast<int>(end_time - start_time);
				}
			}

			double real_fps{ TICKS_PER_SECOND / double(delta_time) };
			std::printf("real_fps: %f\n", real_fps);
		}

		close(window, renderer);

		return 0;
	}
}
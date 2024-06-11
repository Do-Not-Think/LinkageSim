//#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

#include "LinkageObjects.h"
#include "MathConsts.h"
#include "SDLEssentials.h"
#include "Vector.h"

const char* TITLE{ "LinkageSim" };
int screen_width{ 640 };
int screen_height{ 480 };
const Uint64 TICKS_PER_SECOND{ SDL_GetPerformanceFrequency() };
const int ONE_SECOND_MOTOR_PERIOD{ static_cast<int>(TICKS_PER_SECOND) }; //Period in tick
const int FRAME_COUNT{ 100 };                                                                                                                                     

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

		SDL_Color text_color{ 0x00, 0x00, 0x00, 0xff };
		SDL_Color background_color{ 0xff, 0xff, 0xff, 0xff };

		Texture text_texture;



		ButtonStates keyboard[MAX_KEYS];
		ButtonStates mouse[MAX_MOUSEBUTTONS];
		int mouse_x{ 0 }, mouse_y{ 0 };


		bool creating_motor_pos{ false };
		bool creating_motor_arm{ false };

		
		//Linkage Objects
		std::vector<Arm> arms{};
		std::vector<Arm> motors{};
		double scale = 2;

		Arm arm0{};
		arm0.pos = Vector_2d{ scale * (60 + 2 * 20), 200 };
		arm0.angle = -PI / 2.0;
		arm0.length = scale * 2.5 * 20;
		arms.push_back(arm0);

		Arm motor0{};
		motor0.pos = Vector_2d{ scale * 60, 200 };
		motor0.length = scale * 20;
		motor0.angle = -PI / 2.0;
		int motor0_period = 30 * ONE_SECOND_MOTOR_PERIOD;
		motor0.connected_arms.push_back(Index_RodLength{ 0, 50 });//     scale * 2.5 * 20 });
		motors.push_back(motor0);


		int ticks{ 1 };
		int every_nth_tick{ 100 };
		int num_points{ 100 };
		int point_i{ 0 };
		std::vector<Vector_2d> points;


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


			//for fun
			/*static int i{ 0 };

			++i;
			if(i >= 10)
			{
				background_color.r += 3;
				background_color.g += 5;
				background_color.b += -2;
				i = 0;
			}*/
			
			/*
			if(creating_motor_pos || creating_motor_arm)
			{
				//Inversion of the background color
				Uint8 r, g, b;
				SDL_GetRenderDrawColor(renderer, &r, &g, &b, NULL);
				r = 0xff - r;
				g = 0xff - g;
				b = 0xff - b;
				SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
				SDL_Rect rect;
				rect.w = 20;
				rect.h = 20;
				rect.x = mouse_x - rect.w / 2;
				rect.y = mouse_y - rect.h / 2;

				SDL_RenderFillRect(renderer, &rect);
			}
			if(mouse[MOUSE_LEFT].down && creating_motor_pos)
			{
				creating_motor_pos = false;
				creating_motor_arm = true;
			}
			if(mouse[MOUSE_LEFT].down && !creating_motor_arm)
			{
				creating_motor_pos = true;
			}
			if(mouse[MOUSE_LEFT].up)
			{
				if(creating_motor_pos)
				{
					;
				}
				else if(creating_motor_arm)
				{
					creating_motor_arm = false;
				
				}
			}
			*/
			/*
			for(auto& motor : motors)
			{
				std::printf("Motor.pos\t\t %f, %f\n", motor.pos.x, motor.pos.y);
				std::printf("Motor.angle\t\t %f\n", motor.angle * 180 / PI);
				std::printf("Motor.radius\t\t %f\n", motor.radius);
				std::printf("Motor.period\t\t %f\n\n", motor.period);
				
				Uint8 r, g, b;
				r = 0xff - background_color.r;
				g = 0xff - background_color.g;
				b = 0xff - background_color.b;
				SDL_Rect rect;
				rect.w = 20;
				rect.h = 20;
				rect.x = motor.pos.x - rect.w / 2;
				rect.y = motor.pos.y - rect.h / 2;
				SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
				SDL_RenderFillRect(renderer, &rect);
				SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
				

				rect.w = 20;
				rect.h = 20;
				rect.x = motor.pos.x - rect.w / 2 + toCartesianX(motor.radius, motor.angle);
				rect.y = motor.pos.y - rect.h / 2 + toCartesianY(motor.radius, motor.angle);
				SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
				SDL_RenderFillRect(renderer, &rect);
				SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);

				updateMotor(motor, delta_time);
			}*/
			
			
			updateMotors(motors, arms, motor0_period, delta_time);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
			
			Vector_2d pos1{};
			Vector_2d pos2{};
			
			//motor0 arm
			pos1 = motors[0].pos;
			pos2 = motors[0].pos + polarToVector(motors[0].angle, motors[0].length);
			SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);
			//rod connecting motor0 and arm0 
			pos1 = pos2;
			pos2 = arms[0].pos + polarToVector(arms[0].angle, arms[0].length);
			SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);
			//arm0 arm
			pos1 = pos2;
			pos2 = arms[0].pos;
			SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);


			Vector_2d connecting_rod;
			connecting_rod.x = arms[0].pos.x + std::cos(arms[0].angle) * arms[0].length;
			connecting_rod.y = arms[0].pos.y + std::sin(arms[0].angle) * arms[0].length;
			connecting_rod.x -= motors[0].pos.x + std::cos(motors[0].angle) * motors[0].length;
			connecting_rod.y -= motors[0].pos.y + std::sin(motors[0].angle) * motors[0].length;

			connecting_rod *= 2;

			Vector_2d final_pos;
			final_pos.x = connecting_rod.x + motors[0].pos.x + std::cos(motors[0].angle) * motors[0].length;
			final_pos.y = connecting_rod.y + motors[0].pos.y + std::sin(motors[0].angle) * motors[0].length;


			SDL_RenderDrawLine(renderer, motors[0].pos.x + std::cos(motors[0].angle) * motors[0].length, motors[0].pos.y + std::sin(motors[0].angle) * motors[0].length, final_pos.x, final_pos.y);
			
			SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);
			
			if(ticks >= every_nth_tick)
			{
				if(points.size() < num_points)
				{
					points.push_back(final_pos);
					++point_i;
				}
				else
				{
					if(point_i < num_points)
					{
						points[point_i] = final_pos;
					}
					else
					{
						point_i = 0;
					}
				}
				ticks = 1;
			}
			++ticks;


			for(auto& point : points)
			{
				SDL_RenderDrawPoint(renderer, point.x, point.y);
			}


			SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
			
			text_texture.loadFromRenderedText(renderer, text_font, text, text_color, background_color);

			text_texture.render(renderer, 0, 0, NULL);

			fps_sum += fps;
			++fps_sum_i;
			if(fps_sum_i > FRAME_COUNT)
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

			delta_time = static_cast<int>(end_time - start_time);
			fps = TICKS_PER_SECOND / double(delta_time);
		}

		close(window, renderer);

		return 0;
	}
}
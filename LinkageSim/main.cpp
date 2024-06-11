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
const int AVG_FPS_FRAME_COUNT{ 30 };
const double CAMERA_SCALING_FACTOR{ 1.15 };
const double grid_width{ 50 };
const double grid_height{ 50 };
const SDL_Color grid_color{ 0xff, 0x00, 0x00, 0xff };

enum KeysType
{
	F1,
	F11,
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
		//Smoothening of displayed fps
		double displayed_fps{ 0.0 };
		double delta_fps_per_frame{ 0.0 };

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
		
		bool fullscreen{ false };
		/*   Grid   */
		bool grid{ true };

		/*   Camera   */
		Camera camera{};
		camera.width = static_cast<double>(screen_width);
		camera.height = static_cast<double>(screen_height);
		camera.center_pos = Vector_2d{ camera.width / 2.0, camera.height / 2.0 };

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
		int motor0_period = 10.0 * TICKS_PER_SECOND;
		double rod0_length{ 300 };
		motor0.connected_arms.push_back(Index_RodLength{ 1, rod0_length });
		arms.push_back(motor0);

		Arm arm0{};
		arm0.pos = Vector_2d{ 350, 400 };
		arm0.angle = -PI / 2.0;
		arm0.previous_angle = arm0.angle;
		arm0.length = 200;
		arm0.connected_arms.push_back(Index_RodLength{ 2, 50 });
		arms.push_back(arm0);

		Arm arm1{};
		arm1.pos = Vector_2d{ 300, 1000 };
		arm1.angle = -PI / 2.0;
		arm1.previous_angle = arm1.angle;
		arm1.length = 200 + 600;
		arms.push_back(arm1);

		Arm arm2{};
		arm2.pos = arm0.pos;
		arm2.angle = arm0.angle;
		arm2.previous_angle = arm0.angle;
		arm2.length = 300;
		arms.push_back(arm2);



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
					if(SDLK_F1 == e.key.keysym.sym)
					{
						keyboard[F1].down = true;
						keyboard[F1].held = true;
					}
					if(SDLK_F11 == e.key.keysym.sym)
					{
						keyboard[F11].down = true;
						keyboard[F11].held = true;
					}
					
					keysPressed = true;
				}

				if(SDL_KEYUP == e.type && e.key.repeat == 0)
				{
					if(SDLK_F1 == e.key.keysym.sym)
					{
						keyboard[F1].up = true;
						keyboard[F1].held = false;
					}
					if(SDLK_F11 == e.key.keysym.sym)
					{
						keyboard[F11].up = true;
						keyboard[F11].held = false;
					}
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
					if(e.wheel.y > 0)
					{
						camera.width /= CAMERA_SCALING_FACTOR;
						camera.height /= CAMERA_SCALING_FACTOR;
						
						//limiting zooming in
						double ratio{ camera.width / camera.height };

						if (camera.height < 1 && camera.width >= 1)
						{
							camera.height = 1;
							camera.width = ratio * camera.height;
						}
						else if(camera.width < 1)
						{
							camera.width = 1;
							camera.height = 1.0 / ratio * camera.width;
						}
					}
					else if (e.wheel.y < 0)
					{
						camera.width *= CAMERA_SCALING_FACTOR;
						camera.height *= CAMERA_SCALING_FACTOR;
					}
				}
			}
			start_time = SDL_GetPerformanceCounter();

			/*   Toggling fullscreen   */
			if(fullscreen && keyboard[F11].down)
			{
				SDL_SetWindowFullscreen(window, 0);
			}
			if(!fullscreen && keyboard[F11].down)
			{
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
			}

			if(keyboard[F11].down)
			{
				if(fullscreen) fullscreen = false;
				else if(!fullscreen) fullscreen = true;
			}

			

			/*   Toggling grid   */
			if(keyboard[F1].down)
			{
				if(grid) grid = false;
				else if(!grid) grid = true;
			}

			int old_screen_width{ screen_width };
			int old_screen_height{ screen_height };

			SDL_GetRendererOutputSize(renderer, &screen_width, &screen_height);

			if(old_screen_height != screen_height || old_screen_width != screen_width)
			{ 
				camera.height = screen_height;
				camera.width = screen_width;
			}

			TTF_Font* text_font{ TTF_OpenFont(font_path.c_str(), font_size) };

			SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
			SDL_RenderClear(renderer);

			/*   Grabbing and moving the camera   */
			static int clicked_x, clicked_y;
			static Vector_2d clicked_camera_pos;
			if(mouse[MOUSE_LEFT].down)
			{
				clicked_x = mouse_x;
				clicked_y = mouse_y;
				clicked_camera_pos = camera.center_pos;
			}
			if(mouse[MOUSE_LEFT].held)
			{
				Vector_2d move{};
				move.x = mouse_x - clicked_x;
				move.y = mouse_y - clicked_y;
				move.x *=  camera.width / screen_width;
				move.y *=  camera.height / screen_height;
				camera.center_pos = clicked_camera_pos - move;
			}

			if(grid)
			{
				renderGrid(renderer, camera, screen_width, screen_height, grid_width, grid_height, grid_color);
				SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
			}

			updateMotor(arms, 0, motor0_period);
			renderMotorAndConnectedArms(renderer, arms, 0, camera, screen_width, screen_height);
			
			SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
			
			text_texture.loadFromRenderedText(renderer, text_font, text, text_color, background_color);

			text_texture.render(renderer, 0, 0, NULL);

			displayed_fps += delta_fps_per_frame;
			text = std::to_string(displayed_fps);
			fps_sum += fps;
			++fps_sum_i;
			if(fps_sum_i > AVG_FPS_FRAME_COUNT)
			{
				double old_fps_avg{ fps_avg };
				fps_avg = fps_sum / AVG_FPS_FRAME_COUNT;
				delta_fps_per_frame = (fps_avg - old_fps_avg) / AVG_FPS_FRAME_COUNT;
				displayed_fps = old_fps_avg;
				text = std::to_string(displayed_fps);
				fps_sum = 0;
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
				//Busy loop until frame takes as much time as it should
				while(delta_time < FRAME)
				{
					end_time = SDL_GetPerformanceCounter();
					delta_time = static_cast<int>(end_time - start_time);
				}
			}

			double real_fps{ TICKS_PER_SECOND / double(delta_time) };
			//std::printf("real_fps: %f\n", real_fps);
		}

		close(window, renderer);

		return 0;
	}
}
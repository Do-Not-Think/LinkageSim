#pragma once
#include <cmath>
#include <vector>

#include "Vector.h"
#include "MathConsts.h"
#include "SDLEssentials.h"

namespace LinkageSim
{
	const Uint64 TICKS_PER_SECOND{ SDL_GetPerformanceFrequency() };
	const int FRAMERATE{ 120 };
	const int FRAME{ static_cast<int>(TICKS_PER_SECOND / FRAMERATE) }; //Ticks per frame
	const double MOMENTUM{ 1.0 }; //Best kept at 1.0
}


struct Index_RodLength
{
	int index;
	double rod_length;
};

struct Arm
{
	std::vector<Index_RodLength> connected_arms;  //rod length being the distance to arm
	std::vector<Index_RodLength> connected_links;  //rod length beingthe distance to link
	Vector_2d pos{};
	double length{};
	double angle{};
	double previous_angle{};
};

struct Link
{
	std::vector<Index_RodLength> connected_arms;
	double length{};
};



struct Camera
{
	Vector_2d center_pos{};
	double width{};
	double height{};
};

bool isInRect(Vector_2d rect_center_pos, double rect_width, double rect_height, Vector_2d point);

void renderGrid(SDL_Renderer* renderer, const Camera& camera, double screen_width, double screen_height, double grid_width, double grid_height, SDL_Color grid_color = SDL_Color(0xff, 0xff, 0xff, 0xff));

bool updateMotor(std::vector<Arm>& arms, int motor_index, int& period);
bool updateMotor(std::vector<Arm>& arms, int motor_index, int& period, int delta_time); //Using delta_time

void renderMotorAndConnectedArms(SDL_Renderer* renderer, std::vector<Arm>& arms, int preceding_arm_index, const Camera& camera, double screen_width, double screen_height, SDL_Color arm_color = SDL_Color(0xff, 0xff, 0xff, 0xff));
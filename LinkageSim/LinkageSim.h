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
}


struct Index_RodLength
{
	int index;
	double rod_length;
};

struct Arm
{
	std::vector<Index_RodLength> connected_arms;
	Vector_2d pos{};
	double length{};
	double angle{};
	double previous_angle{};
};

bool updateMotor(std::vector<Arm>& arms, int motor_index, int& period);
bool updateMotor(std::vector<Arm>& arms, int motor_index, int& period, int delta_time); //Using delta_time
#pragma once
#include <vector>

#include "Vector.h"
#include "MathConsts.h"

struct Index_Distance
{
	unsigned int index;
	double distance; //connecting arm/ rod length
};

struct Arm
{
	std::vector<Index_Distance> connected_arms;
	Vector_2d pos{};
	double arm_length{};
	double angle{};
	double displaced_angle{};
};

struct Motor
{
	std::vector<Index_Distance> connected_arms;
	Vector_2d pos{};
	double arm_length{};
	double angle{};
	double period{};
	bool reversed{ false };
};

bool updateMotors(std::vector<Motor>& motors, std::vector<Arm>& arms, int delta_time);
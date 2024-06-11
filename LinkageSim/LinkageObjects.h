#pragma once
#include <cmath>
#include <vector>

#include "Vector.h"
#include "MathConsts.h"

enum MotorState;

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

bool updateMotors(std::vector<Arm>& motors, std::vector<Arm>& arms, int period, int delta_time);
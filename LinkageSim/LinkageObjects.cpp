#include "LinkageObjects.h"

double normalizedAngle(double angle)
{
	while(angle > PI)
		angle = angle - 2 * PI;
	while(angle <= - PI)
		angle = angle + 2 * PI;
	return angle;
}

bool circleCircleIntersection(const Vector_2d& pos1, double radius1, const Vector_2d& pos2, double radius2, Vector_2d& intersection1, Vector_2d& intersection2)
{
	Vector_2d CenterCenter{ pos2 - pos1 };

	double distanceSqr{ lengthSqr(CenterCenter) };


	if(distanceSqr > (radius1 + radius2) * (radius1 + radius2))
	{
		return false;
	}
	if(distanceSqr < (radius1 - radius2) * (radius1 - radius2))
	{
		return false;
	}

	double inv_distance = 1.0 / std::sqrt(distanceSqr);

	//distance to point_A where the lines: between circle centers
	//and line that goes through the circle intersection cross
	double a = (radius1 * radius1 - radius2 * radius2 + distanceSqr) * 0.5 * inv_distance;

	//normalize CenterCenter
	CenterCenter = CenterCenter * inv_distance;

	//distance from point_A to intersection points
	double h = std::sqrt(radius1 * radius1 - a * a);

	Vector_2d point_A{};
	point_A = a * CenterCenter + pos1;

	CenterCenter = getPerpendicularVector(CenterCenter);
	intersection1 = point_A + h * CenterCenter;
	intersection2 = point_A - h * CenterCenter;

	return true;
}

void updateAngle(Motor& motor, int delta_time)
{
	if(motor.period != 0)
	{
		double delta_angle = 2.0 * PI * delta_time / motor.period;
		if(delta_angle > PI / 2.0)
			std::printf("delta_angle > pi/2!: %f * 2pi\n", delta_time / motor.period);
		motor.angle += ( 1 - 2 * motor.reversed) * delta_angle;
	}
	//not necessary but ensures no undue overflow or underflow
	motor.angle = normalizedAngle(motor.angle);
}

bool updateArms(std::vector<Arm>& arms, int this_index)
{
	bool success{ true };

	for(int i{ 0 }; i < arms[this_index].connected_arms.size(); ++i)
	{
		unsigned int index{ arms[this_index].connected_arms[this_index].index };
		double distance_to_arm{ arms[this_index].connected_arms[this_index].distance };

		Vector_2d arm{};
		arms[index].angle = normalizedAngle(arms[index].angle);
		arm.x = cos(arms[index].angle) * arms[index].arm_length;
		arm.y = sin(arms[index].angle) * arms[index].arm_length;

		Vector_2d arm_new_1{};
		Vector_2d arm_new_2{};
		if(!circleCircleIntersection(arms[this_index].pos, distance_to_arm, arms[index].pos, arms[index].arm_length, arm_new_1, arm_new_2))
		{
			success = false;
			std::printf("updateArms: failed to update arm: %d\n", index);
		}
		else
		{
			//Update each arm connected to arms[index]
			success = updateArms(arms, index);
		}
	}

	return success;
}

bool updateMotors(std::vector<Motor>& motors, std::vector<Arm>& arms, int delta_time)
{
	bool success{ true };

	for(int i{ 0 }; i < motors.size(); ++i)
	{
		updateAngle(motors[i], delta_time);

		Vector_2d motor_arm_pos{};
		motor_arm_pos.x = motors[i].pos.x + std::cos(motors[i].angle) * motors[i].arm_length;
		motor_arm_pos.y = motors[i].pos.y + std::sin(motors[i].angle) * motors[i].arm_length;
		//Update every conneceted arm
		for(int j{ 0 }; j < motors[i].connected_arms.size(); ++j)
		{
			unsigned int index{ motors[i].connected_arms[j].index };
			double distance_to_arm{ motors[i].connected_arms[j].distance };

			Vector_2d arm{};
			arm.x = cos(arms[index].angle) * arms[index].arm_length;
			arm.y = sin(arms[index].angle) * arms[index].arm_length;

			Vector_2d new_arm_1{};
			Vector_2d new_arm_2{};
			if(!circleCircleIntersection(motor_arm_pos, distance_to_arm, arms[index].pos, arms[index].arm_length, new_arm_1, new_arm_2))
			{
				success = false;
				std::printf("updateArms: failed to update arm: %d\n", index);

			}
			else
			{
				new_arm_1 = new_arm_1 - arms[index].pos;
				double new_displaced_angle_1 = normalizedAngle(std::atan2(new_arm_1.y, new_arm_1.x) - arms[index].angle);

				new_arm_2 = new_arm_2 - arms[index].pos;
				double new_displaced_angle_2 = normalizedAngle(std::atan2(new_arm_2.y, new_arm_2.x) - arms[index].angle);

				arms[index].displaced_angle = normalizedAngle(arms[index].displaced_angle);

				std::printf("displaced_angle       %f\n", arms[index].displaced_angle * 180 / PI);
				std::printf("new_displaced_angle_1 %f\n", new_displaced_angle_1 * 180 / PI);
				std::printf("new_displaced_angle_2 %f\n\n", new_displaced_angle_2 * 180 / PI);

				if(std::abs(arms[index].displaced_angle - new_displaced_angle_1) < std::abs(arms[index].displaced_angle - new_displaced_angle_2))
				{
					arms[index].displaced_angle = new_displaced_angle_1;
				}
				else
				{
					arms[index].displaced_angle = new_displaced_angle_2;
				}

				arms[index].angle += arms[index].displaced_angle;

				//Update each arm connected to arms[index]
				success = updateArms(arms, index);
			}
			
		}
	}

	return success;
}
#include "LinkageSim.h"

using namespace LinkageSim;

bool isInRect(Vector_2d rect_center_pos, double rect_width, double rect_height, Vector_2d point)
{
	rect_center_pos.x -= rect_width / 2.0;
	rect_center_pos.y -= rect_height / 2.0;
	return (point.x >= rect_center_pos.x && point.x <= rect_center_pos.x + rect_width) && (point.y >= rect_center_pos.y && point.y <= rect_center_pos.y + rect_height);
}

Vector_2d cameraTranformedPos(const Camera& camera, double screen_width, double screen_height, Vector_2d pos)
{
	pos -= camera.center_pos;
	pos.x *= screen_width / camera.width;
	pos.y *= screen_height /camera.height;
	pos.x += screen_width / 2.0;
	pos.y += screen_height / 2.0;

	return pos;
}

void renderGrid(SDL_Renderer* renderer, const Camera& camera, double screen_width, double screen_height, double grid_width, double grid_height, SDL_Color grid_color)
{
	SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, grid_color.a);

	Vector_2d pos1, pos2;
	int gridlines{};
	int starting_gridline{};

	//Generating vertical lines
	pos1.y = camera.center_pos.y - 1 * camera.height - grid_height; //Should go atleast one grid_width away, to ensure the grid stays rendered even when zoomed in
	pos2.y = camera.center_pos.y + 1 * camera.height + grid_height;

	gridlines = 2 + static_cast<int>(1.0 * camera.width) / static_cast<int>(grid_width); //There should be atleast two grids, to ensure the grid stays rendered even when zoomed in
	starting_gridline = static_cast<int>(camera.center_pos.x - .5 * camera.width) / static_cast<int>(grid_width);
	for(int i{ 0 }; i < gridlines; ++i)
	{
		pos1.x = (starting_gridline + i) * grid_width;
		pos2.x = pos1.x;

		Vector_2d screen_pos1 = cameraTranformedPos(camera, screen_width, screen_height, pos1);
		Vector_2d screen_pos2 = cameraTranformedPos(camera, screen_width, screen_height, pos2);

		SDL_RenderDrawLine(renderer, screen_pos1.x, screen_pos1.y, screen_pos2.x, screen_pos2.y);
	}

	//Generating horizontal lines
	pos1.x = camera.center_pos.x - 1 * camera.width - grid_width;
	pos2.x = camera.center_pos.x + 1 * camera.width + grid_width;

	gridlines = 2 + static_cast<int>(1.0 * camera.height) / static_cast<int>(grid_height);
	starting_gridline = static_cast<int>(camera.center_pos.y - .5 * camera.height) / static_cast<int>(grid_height);
	for(int i{ 0 }; i < gridlines; ++i)
	{
		pos1.y = (starting_gridline + i) * grid_height;
		pos2.y = pos1.y;

		Vector_2d screen_pos1 = cameraTranformedPos(camera, screen_width, screen_height, pos1);
		Vector_2d screen_pos2 = cameraTranformedPos(camera, screen_width, screen_height, pos2);

		SDL_RenderDrawLine(renderer, screen_pos1.x, screen_pos1.y, screen_pos2.x, screen_pos2.y);
	}
}

double normalizedAngle(double angle)
{
	while(angle > PI)
		angle = angle - 2 * PI;
	while(angle <= -PI)
		angle = angle + 2 * PI;
	return angle;
}

void updateAngle(Arm& motor, int period)
{
	if(period != 0)
	{
		motor.previous_angle = motor.angle;
		motor.angle += 2.0 * PI * FRAME / static_cast<double>(period);
	}
	//motor.angle = normalizedAngle(motor.angle);
}
//Using delta_time
void updateAngle(Arm& motor, int period, int delta_time)
{
	if(period != 0)
	{
		motor.previous_angle = motor.angle;
		motor.angle += 2.0 * PI * delta_time / static_cast<double>(period);
	}
	//motor.angle = normalizedAngle(motor.angle);
}

//Return 1 means a squash/ stretch occured, reverse!
bool updateArm(std::vector<Arm>& arms, int arm_index, double rod_length, int preceding_arm_index)
{
	//Extract data
	Vector_2d preceding_arm_pos{ arms[preceding_arm_index].pos };
	double preceding_arm_angle{ arms[preceding_arm_index].angle };
	double preceding_arm_length{ arms[preceding_arm_index].length };
	double preceding_arm_previous_angle{ arms[preceding_arm_index].previous_angle };
	Vector_2d preceding_arm_end_pos{ preceding_arm_pos + polarToVector(preceding_arm_angle, preceding_arm_length) };

	Vector_2d arm_pos{ arms[arm_index].pos };
	double arm_angle{ arms[arm_index].angle };
	double arm_length{ arms[arm_index].length };
	double arm_previous_angle{ arms[arm_index].previous_angle };




	//What we will be solving for
	Vector_2d new_arm_end_pos{};




	//Make a new coordinate system centered around arm_pos, by shifting everything
	preceding_arm_end_pos -= arm_pos;
	arm_pos = Vector_2d(0.0, 0.0);
	
	//Rotate everything by the angle vector preceding_arm_end_pos has
	//preceding_arm_end_pos will lay flat on the x axis
	double coord_sys_angle{ std::atan2(preceding_arm_end_pos.y, preceding_arm_end_pos.x) };
	preceding_arm_end_pos = rotatedVectorBackwards(preceding_arm_end_pos, preceding_arm_end_pos);		//arm_pos.y == 0
	arm_angle = arm_angle - coord_sys_angle;

	double squash_distance{  std::abs(rod_length - arm_length) };
	double stretch_distance{          rod_length + arm_length  };
	if(preceding_arm_end_pos.x < squash_distance)
	{
		//Squashing
		std::printf("\nupdateArm: preceding_arm_index %d, arm_index %d, preceding_arm_end_pos is too close to arm_pos!\ndistance between them: %f\nminimum distance: %f\n", preceding_arm_index, arm_index, distance(preceding_arm_end_pos, arm_pos), squash_distance);

		//Go back a step
		arms[preceding_arm_index].angle = arms[preceding_arm_index].previous_angle;
		arms[arm_index].angle = arms[arm_index].previous_angle;

		//Reverse
		return 1;
	}
	else if(preceding_arm_end_pos.x > stretch_distance)
	{
		//Stretching
		std::printf("\tupdateArm: preceding_arm_index %d, arm_index %d, preceding_arm_end_pos is too far from arm_pos!\ndistance between them: %f\nmaximum distance: %f\n", preceding_arm_index, arm_index, distance(preceding_arm_end_pos, arm_pos), stretch_distance);

		//Go back a step
		arms[preceding_arm_index].angle = arms[preceding_arm_index].previous_angle;
		arms[arm_index].angle = arms[arm_index].previous_angle;

		//Reverse
		return 1;
	}
	else
	{
		//We can now solve for new_arm_end_pos, using circle-circle intersection
		new_arm_end_pos.x = arm_length * arm_length - rod_length * rod_length + preceding_arm_end_pos.x * preceding_arm_end_pos.x;
		new_arm_end_pos.x /= 2.0 * preceding_arm_end_pos.x;

		new_arm_end_pos.y = std::sqrt(arm_length * arm_length - new_arm_end_pos.x * new_arm_end_pos.x);

		//Two +- solutions for new_arm_end_pos.y
		double new_angle_1 = std::atan2(+new_arm_end_pos.y, new_arm_end_pos.x);
		double new_angle_2 = std::atan2(-new_arm_end_pos.y, new_arm_end_pos.x);

		double delta_angle_1 = new_angle_1 - arm_angle;
		double delta_angle_2 = new_angle_2 - arm_angle;

		double previous_delta_angle{ arms[arm_index].angle - arms[arm_index].previous_angle };

		double momentum = MOMENTUM;
		//check which one makes a delta_angle closer the the last_delta_angle
		if(std::abs(normalizedAngle(momentum * previous_delta_angle - delta_angle_1)) < std::abs(normalizedAngle(momentum * previous_delta_angle - delta_angle_2)))
		{
			//delta_angle_1 is closer
			previous_delta_angle = delta_angle_1;
			arm_angle = new_angle_1;
		}
		else
		{
			//delta_angle_2 is closer;
			previous_delta_angle = delta_angle_2;
			arm_angle = new_angle_2;
		}
		arm_previous_angle = arms[arm_index].angle;
		arm_angle += coord_sys_angle;

		//Update any subsequent arms
		for(int i{ 0 }; i < arms[arm_index].connected_arms.size(); ++i)
		{
			if(updateArm(arms, arms[arm_index].connected_arms[i].index, arms[arm_index].connected_arms[i].rod_length, arm_index))
			{
				return 1;
			}
		}

		arms[arm_index].previous_angle = arm_previous_angle;
		arms[arm_index].angle = arm_angle;

		return 0;
	}
}

bool updateLink(std::vector<Link>& links, int link_index, double rod_length, std::vector<Arm>& arms, int preceding_arm_index, Vector_2d passthrough)
{
	return 1;
}

bool updateMotor(std::vector<Arm>& arms, int motor_index, int& period)
{
	bool success{ true };

	for(int i{ 0 }; i < arms[motor_index].connected_arms.size(); ++i)
	{
		if(updateArm(arms, arms[motor_index].connected_arms[i].index, arms[motor_index].connected_arms[i].rod_length, motor_index))
		{
			success = false;
			//Reverse
			period = -period;

			break;
		}
	}

	updateAngle(arms[motor_index], period);

	return success;
}
//Using delta_time
bool updateMotor(std::vector<Arm>& arms, int motor_index, int& period, int delta_time)
{
	bool success{ true };

	for(int i{ 0 }; i < arms[motor_index].connected_arms.size(); ++i)
	{
		if(updateArm(arms, arms[motor_index].connected_arms[i].index, arms[motor_index].connected_arms[i].rod_length, motor_index))
		{
			success = false;
			//Reverse
			period = -period;

			break;
		}
	}

	updateAngle(arms[motor_index], period);

	return success;
}

void renderAndConnectArms(SDL_Renderer* renderer, std::vector<Arm>& arms, int arm_index, int preceding_arm_index, const Camera& camera, double screen_width, double screen_height, SDL_Color arm_color)
{
	Vector_2d preceding_arm_pos{ arms[preceding_arm_index].pos };
	double preceding_arm_angle{ arms[preceding_arm_index].angle };
	double preceding_arm_length{ arms[preceding_arm_index].length };
	Vector_2d preceding_arm_end_pos{ preceding_arm_pos + polarToVector(preceding_arm_angle, preceding_arm_length) };

	Vector_2d arm_pos{ arms[arm_index].pos };
	double arm_angle{ arms[arm_index].angle };
	double arm_length{ arms[arm_index].length };
	Vector_2d arm_end_pos{ arm_pos + polarToVector(arm_angle, arm_length) };

	SDL_SetRenderDrawColor(renderer, arm_color.r, arm_color.g, arm_color.b, arm_color.a);

	Vector_2d pos1{};
	Vector_2d pos2{};

	pos1 = cameraTranformedPos(camera, screen_width, screen_height, preceding_arm_pos);
	pos2 = cameraTranformedPos(camera, screen_width, screen_height, preceding_arm_end_pos);
	SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);

	pos1 = cameraTranformedPos(camera, screen_width, screen_height, arm_pos);
	pos2 = cameraTranformedPos(camera, screen_width, screen_height, arm_end_pos);
	SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);

	pos1 = cameraTranformedPos(camera, screen_width, screen_height, arm_end_pos);
	pos2 = cameraTranformedPos(camera, screen_width, screen_height, preceding_arm_end_pos);
	SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);

	for(int i{ 0 }; i < arms[arm_index].connected_arms.size(); ++i)
	{
		renderAndConnectArms(renderer, arms, arms[arm_index].connected_arms[i].index, arm_index, camera, screen_width, screen_height, arm_color);
	}
}

void renderMotorAndConnectedArms(SDL_Renderer* renderer, std::vector<Arm>& arms, int motor_index, const Camera& camera, double screen_width, double screen_height, SDL_Color arm_color)
{
	for(int i{ 0 }; i < arms[motor_index].connected_arms.size(); ++i)
	{
		renderAndConnectArms(renderer, arms, arms[motor_index].connected_arms[i].index, motor_index, camera, screen_width, screen_height, arm_color);
	}
}
#include "LinkageObjects.h"

//#define NO_DELTA_TIME
#define SET_DELTA_TIME 10'000

enum MotorState
{
	MOTORSTATE_DEFAULT,
	MOTORSTATE_REVERSE,
	MOTORSTATE_STOP,
	MAX_MOTORSTATES
};

/*			Naming scheme:				*/
// Different pos:
// previous		Previous arm
// beginning	Beginning of (connecting) rod
// end			End of (connecting) rod
// next			Next arm
// length_...	Length of ...

/*
bool updateMotors(std::vector<Motor>& motors, std::vector<Arm>& arms, int delta_time)
{
	bool success{ true };

	for(int i{ 0 }; i < motors.size(); ++i)
	{
		updateAngle(motors[i], delta_time);

		Vector_2d previous{ motors[i].pos };
		Vector_2d beginning{ motors[i].pos + polarToVector(motors[i].angle, motors[i].length) };

		//Update every conneceted arm
		for(int j{ 0 }; j < motors[i].connected_arms.size(); ++j)
		{
			//Finding end_rod
			int next_index{ motors[i].connected_arms[j].index };
			double length_rod{ motors[i].connected_arms[j].length_rod };

			Vector_2d next{ arms[next_index].pos };

			Vector_2d coordinate_system_pos{ beginning }; //now beginning is at (0,0) of our coord system
			beginning -= coordinate_system_pos;
			next -=		 coordinate_system_pos;

			double coordinate_system_angle{ std::atan2(next.y, next.x) };
			std::printf("coords angle: %f\n", coordinate_system_angle * 180.0 / PI);

			next = rotatedVectorBackwards(next, next); //now next.y == 0, and next.x is the distance between beginning and next


			if(next.x * next.x < (length_rod - arms[next_index].length) * (length_rod - arms[next_index].length))
			{
				std::printf("");//Squashing
			}
			else if(next.x * next.x > (length_rod + arms[next_index].length) * (length_rod + arms[next_index].length))
			{
				std::printf("");//Stretching
			}
			else
			{
				double end_x{ (length_rod * length_rod - arms[next_index].length * arms[next_index].length + next.x * next.x) / (2.0 * next.x) };
				double end_y_1{ +std::sqrt(length_rod * length_rod - end_x * end_x) };
				double end_y_2{ -std::sqrt(length_rod * length_rod - end_x * end_x) };
				
			
				//next_angle inside our coord system
				double next_angle = arms[next_index].angle - coordinate_system_angle;


				//Check which solution is more aligned with previos_delta_angle
				double angle_1 = std::atan2(end_y_1, end_x);
				double delta_angle_1 = angle_1 - next_angle;

				double angle_2 = std::atan2(end_y_2, end_x);
				double delta_angle_2 = angle_2 - next_angle;
				
				//THIS NEEDS TESTING
				if(normalizedAngle(delta_angle_1 - arms[next_index].last_delta_angle) < normalizedAngle(delta_angle_2 - arms[next_index].last_delta_angle))
				{
					arms[next_index].last_delta_angle = normalizedAngle(delta_angle_1);
					next_angle = angle_1;
				}
				else
				{
					arms[next_index].last_delta_angle = normalizedAngle(delta_angle_2);
					next_angle = angle_2;
				}

				//next_angle outside our coord system
				arms[next_index].angle = normalizedAngle(next_angle - coordinate_system_angle);

				//updateArms();
			}

		}
	}

	return success;
}*/

void updateAngle(Arm& motor, int period, int delta_time);
double normalizedAngle(double angle);
MotorState updateMotorArms(std::vector<Arm>& motors, std::vector<Arm>& arms, int motor_index);

bool updateMotors(std::vector<Arm>& motors, std::vector<Arm>& arms, int period, int delta_time)//change period to std::vector<int> period
{
	bool success{ true };

	//Note to self: remove statics
	//I propably should be using statics, but they are sooo convenient here
	static MotorState motor_state{ MOTORSTATE_DEFAULT };
	static bool reverse{ false };

	for(int i{ 0 }; i < motors.size(); ++i)
	{
		
		if(motor_state == MOTORSTATE_REVERSE)
		{
			std::printf("\nReversed!\n");
			reverse = !reverse;
			motor_state = MOTORSTATE_DEFAULT;
		}
		if(reverse)
			updateAngle(motors[i], -period, delta_time);
		else
			updateAngle(motors[i], period, delta_time);

		motor_state = updateMotorArms(motors, arms, i);

		if(MOTORSTATE_DEFAULT != motor_state)
			success = false;
	}

	return success;
}

void updateAngle(Arm& motor,int period, int delta_time)
{
	if(period != 0)
	{
#ifndef NO_DELTA_TIME
		motor.previous_angle = motor.angle;
		motor.angle += 2.0 * PI * delta_time / static_cast<double>(period);
#endif
#ifdef NO_DELTA_TIME
		motor.previous_angle = motor.angle; 
		motor.angle += 2.0 * PI * SET_DELTA_TIME / static_cast<double>(period);
#endif
	}
	motor.angle = normalizedAngle(motor.angle);
}

double normalizedAngle(double angle)
{
	while(angle > PI)
		angle = angle - 2 * PI;
	while(angle <= -PI)
		angle = angle + 2 * PI;
	return angle;
}

MotorState updateMotorArms(std::vector<Arm>& motors, std::vector<Arm>& arms, int motor_index)
{
	MotorState motor_state{ MOTORSTATE_DEFAULT };
	

	Vector_2d motor_pos{ motors[motor_index].pos };
	double motor_angle{ motors[motor_index].angle };
	double motor_length{ motors[motor_index].length };
	double motor_previous_angle{ motors[motor_index].previous_angle };
	Vector_2d motor_end_pos{ motor_pos + polarToVector(motor_angle, motor_length) };
	
	for(int i{ 0 }; i < motors[motor_index].connected_arms.size(); ++i)
	{
		int arm_index{ motors[motor_index].connected_arms[i].index };
		double rod_length{ motors[motor_index].connected_arms[i].rod_length };
		
		Vector_2d new_arm_end_pos{};
		Vector_2d arm_pos{ arms[arm_index].pos };
		double arm_angle{ arms[arm_index].angle };
		double arm_length{ arms[arm_index].length };
		double arm_previous_angle{ arms[arm_index].previous_angle };


		//Make a new coordinate system centered around arm_pos
		motor_end_pos -= arm_pos;
		arm_pos = Vector_2d(0.0, 0.0);

		//rotate everything by the angle vector arm_pos has
		//arm_pos will lay flat on the x axis
		double coord_sys_angle{ std::atan2(motor_end_pos.y, motor_end_pos.x) };
		motor_end_pos = rotatedVectorBackwards(motor_end_pos, motor_end_pos);		//arm_pos.y == 0
		arm_angle = normalizedAngle(arm_angle - coord_sys_angle);


		double squash_distance{ std::abs(rod_length - arm_length) };
		double strech_distance{          rod_length + arm_length };
		if(motor_end_pos.x < squash_distance)
		{
			//Squashing
			std::printf("\nupdateMotorArms: motors[%d], arm[%d], motor_end_pos is too close to arm_pos!\ndistance between them: %f\nminimum distance: %f\n", motor_index, arm_index, distance(motor_end_pos, arm_pos), squash_distance);
			motor_state = MOTORSTATE_REVERSE;
			
			//Go back a step
			motors[motor_index].angle = motors[motor_index].previous_angle;
			//motor_angle = motors[motor_index].angle;
			arms[arm_index].angle = arms[arm_index].previous_angle;
			//arm_angle = arms[arm_index].angle;
			/*
			//Reset everything
			motor_end_pos = motor_pos + polarToVector(motor_angle, motor_length);
			arm_pos = arms[arm_index].pos;
			arm_angle = arms[arm_index].angle;
			//Make a new coordinate system centered around arm_pos
			motor_pos -= arm_pos;
			motor_end_pos -= arm_pos;
			arm_pos = Vector_2d(0.0, 0.0);

			//Rotate everything by the angle that vector motor_pos has
			coord_sys_angle = std::atan2(motor_pos.y, motor_pos.x);
			motor_end_pos = rotatedVectorBackwards(motor_end_pos, motor_pos);
			motor_pos = rotatedVectorBackwards(motor_pos, motor_pos);
			arm_angle = normalizedAngle(arm_angle - coord_sys_angle);
			motor_angle = normalizedAngle(motor_angle - coord_sys_angle);

			//Check if there are solutions
			if(squash_distance > motor_pos.x + motor_length)
			{
				//connecting rod too long
				//motor and arm too close
				//std::printf("\tupdateMotorArms: motors[%d], arm[%d], motor_pos is too close to arm_pos!\ndistance between them: %f\nminimum distance: %f\n", motor_index, arm_index, distance(motor_pos, arm_pos), squash_distance - motor_length);
				std::printf("updateMotorArms: motors[%d], arm[%d], connecting rod too long!\nrod_length: %f\n max length: %f\n", motor_index, arm_index, rod_length, distance(motor_pos, arm_pos) + motor_length - arm_length);
			}
			else if(squash_distance < motor_pos.x - motor_length)
			{
				//connecting rod too short
				//motor and arm too far
				//std::printf("\tupdateMotorArms: motors[%d], arm[%d], motor_pos is too far from arm_pos!\ndistance between them: %f\nmaximum distance: %f\n", motor_index, arm_index, distance(motor_pos, arm_pos), squash_distance + motor_length);
				std::printf("updateMotorArms: motors[%d], arm[%d], connecting rod too short!\nrod_length: %f\n min length: %f\n", motor_index, arm_index, rod_length, distance(motor_pos, arm_pos) - motor_length + arm_length);
			}
			else
			{
				//Now we can solve for new motor_end_pos, such that the distance is exacly equal rod_length + arm_length
				motor_end_pos.x = motor_length * motor_length - squash_distance * squash_distance - motor_pos.x * motor_pos.x;
				motor_end_pos.x /= -2.0 * motor_pos.x;

				//Two +- solutions for motor_end_pos.y
				motor_end_pos.y = std::sqrt(squash_distance * squash_distance - motor_end_pos.x * motor_end_pos.x);

				new_arm_end_pos = arm_length * normalizedVector(motor_end_pos);



				double new_angle_1 = std::atan2(+(motor_end_pos - motor_pos).y, (motor_end_pos - motor_pos).x);
				double new_angle_2 = std::atan2(-(motor_end_pos - motor_pos).y, (motor_end_pos - motor_pos).x);// this -> (= 3 * PI / 2.0 - new_angle_1;) doesnt work all the time, for some reason, idc

				double delta_angle_1 = new_angle_1 - motor_angle;
				double delta_angle_2 = new_angle_2 - motor_angle;


				
				//check which one makes a delta_angle closer the the last_delta_angle
				if(std::abs(normalizedAngle(motor_last_delta_angle - delta_angle_1)) < std::abs(normalizedAngle(motor_last_delta_angle - delta_angle_2)))
				{
					//delta_angle_1 is closer
					motor_last_delta_angle = delta_angle_1;
					motor_angle = new_angle_1;				
					//postive solution is true
				}
				else
				{
					//delta_angle_2 is closer
					motor_last_delta_angle = delta_angle_2;
					motor_angle = new_angle_2;

					//negative solution is true
					new_arm_end_pos.y = -new_arm_end_pos.y;
				}
				arm_angle = std::atan2(new_arm_end_pos.y, new_arm_end_pos.x);

				motors[motor_index].angle = normalizedAngle(motor_angle + coord_sys_angle);
				motors[motor_index].last_delta_angle = motor_last_delta_angle;
				arms[arm_index].angle = normalizedAngle(arm_angle + coord_sys_angle);
				
			}*/
		}
		else if(motor_end_pos.x > strech_distance)
		{
			//Streching
			std::printf("\tupdateMotorArms: motors[%d], arm[%d], motor_end_pos is too far from arm_pos!\ndistance between them: %f\nmaximum distance: %f\n", motor_index, arm_index, distance(motor_end_pos, arm_pos), squash_distance);
			motor_state = MOTORSTATE_REVERSE;

			//Go back a step
			motors[motor_index].angle = motors[motor_index].previous_angle;
			//motor_angle = motors[motor_index].angle;
			arms[arm_index].angle = arms[arm_index].previous_angle;
			//arm_angle = arms[arm_index].angle;
		}
		else
		{
			//We can now solve for end_pos
			//new_arm_end_pos has to stay a fixed distance away from motor_arm_pos and a fixed distance away from motor_end_pos
			new_arm_end_pos.x = arm_length * arm_length - rod_length * rod_length + motor_end_pos.x * motor_end_pos.x;
			new_arm_end_pos.x /= 2.0 * motor_end_pos.x;

			//Two +- solutions for new_arm_end_pos.y
			new_arm_end_pos.y = std::sqrt(arm_length * arm_length - new_arm_end_pos.x * new_arm_end_pos.x);

			double new_angle_1 = std::atan2(+new_arm_end_pos.y, new_arm_end_pos.x);
			double new_angle_2 = std::atan2(-new_arm_end_pos.y, new_arm_end_pos.x);

			double delta_angle_1 = new_angle_1 - arm_angle;
			double delta_angle_2 = new_angle_2 - arm_angle;
			
			double previous_delta_angle{ arms[arm_index].angle - arms[arm_index].previous_angle };

			double momentum = 1.0;
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
			arms[arm_index].previous_angle = arms[arm_index].angle;
			arms[arm_index].angle = normalizedAngle(arm_angle + coord_sys_angle);

			//UpdateArms()
		}
	}

	return motor_state;
}


bool updateArms(std::vector<Arm>& arms, int this_index)
{
	return true;
}
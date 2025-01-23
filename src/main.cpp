#include "main.h"

// Global instances
pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::MotorGroup left_mg({1, -2, 3});

// (1, 2, 3) tried
// (1, 2, -3) tried
// (1, -2, 3) tried
// (1, -2, -3)
// (-1, -2, -3)
// 1 and 2 go same direction

// pros::MotorGroup left_mg({1});

pros::MotorGroup right_mg({-4, 5, -6});
pros::MotorGroup intake_mg({19});		// Controlled by right side buttons on controller
pros::MotorGroup conveyor_mg({-20});	// Controlled by left side buttoms
pros::ADIDigitalOut mogo_solenoid('H'); // Controlled by A and B buttons

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button()
{
	static bool pressed = false;
	pressed = !pressed;
	if (pressed)
	{
		// For running auton later
		// selected_autonomous = (selected_autonomous + 1) % NUM_AUTONOMOUS_ROUTINES;
		// pros::lcd::set_text(2, "Auton: " + std::to_string(selected_autonomous));

		pros::lcd::set_text(2, "I was pressed!");
	}
	else
	{
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize()
{
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");

	pros::lcd::register_btn1_cb(on_center_button);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

// HELPER FUNCTIONS
void drive_forward(int speed, int duration_ms)
{
	left_mg.move(speed);
	right_mg.move(speed);
	pros::delay(duration_ms);
	left_mg.move(0);
	right_mg.move(0);
}

void turn(int speed, int duration_ms)
{
	left_mg.move(speed);
	right_mg.move(-speed);
	pros::delay(duration_ms);
	left_mg.move(0);
	right_mg.move(0);
}

void control_intake(int speed, int duration_ms)
{
	intake_mg.move(speed);
	pros::delay(duration_ms);
	intake_mg.move(0);
}

void control_conveyor(int speed, int duration_ms)
{
	conveyor_mg.move(speed);
	pros::delay(duration_ms);
	conveyor_mg.move(0);
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous()
{
	// start really simple: https://www.youtube.com/shorts/ouInSnVIrC0
	// path: drive forward and score immediately
	// rotate
	// score a ring on the middle wall stake on our side
	// rotate again and drive forward
	// try to end by contacting the ladder

	// start with mogo mech facing fowrward

	int left_move = master.get_analog(ANALOG_LEFT_Y);
	int right_move = master.get_analog(ANALOG_RIGHT_Y);
	
	left_mg.move(127); // Sets left motor voltage
	right_mg.move(127);

	// mogo_solenoid.set_value(true);
	conveyor_mg.move(300); // try to score on mogo

	// still need to add:
	// turn about 330 degree to the right
	// drive forward toward wall stake
	// score
	// drive backwards until you hit the ladder

	pros::delay(1000); // Run for 20 ms then update
}

void arcade_drive()
{
	int forward = master.get_analog(ANALOG_LEFT_Y);
	int turn = master.get_analog(ANALOG_RIGHT_X);

	int left_speed = forward + turn;
	int right_speed = forward - turn;

	left_speed = std::max(-127, std::min(left_speed, 127));
	right_speed = std::max(-127, std::min(right_speed, 127));

	left_mg.move(left_speed);
	right_mg.move(right_speed);

	pros::delay(20);
}

void tank_drive()
{
	int left_move = master.get_analog(ANALOG_RIGHT_Y);
	int right_move = master.get_analog(ANALOG_LEFT_Y); // ANALOG_LEFT_Y
	left_mg.move(left_move);						   // Sets left motor voltage
	right_mg.move(right_move);
	printf("hello world");
	pros::delay(20); // Run for 20 ms then update
}
/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol()
{
	bool mogo_active = false;

	while (true)
	{
		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
						 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
						 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0); // Prints status of the emulated screen LCDs

		// Arcade control scheme: one drives, one turns
		// int dir = master.get_analog(ANALOG_LEFT_Y);    // Gets amount forward/backward from left joystick
		// int turn = master.get_analog(ANALOG_RIGHT_X);  // Gets the turn left/right from right joystick
		// left_mg.move(dir - turn);                      // Sets left motor voltage
		// right_mg.move(dir + turn);                     // Sets right motor voltage

		// tank drive: each joystick controls each side
		// tank_drive();
		// arcade drive - right joystick controls turning, left joystick  controls forward/backward
		arcade_drive();

		// ** Intake and conveyor **

		// intake forward
		if (master.get_digital(DIGITAL_R1))
		{
			intake_mg.move(300);
		}

		// intake reverse
		else if (master.get_digital(DIGITAL_R2))
		{
			intake_mg.move(-300);
		}
		// stop intake
		else
		{
			intake_mg.move(0);
		}

		// conveyor forward
		if (master.get_digital(DIGITAL_L1))
		{
			conveyor_mg.move(100);
		}
		// conveyor reverse
		else if (master.get_digital(DIGITAL_L2))
		{
			conveyor_mg.move(-100);
		}
		// stop the conveyor
		else
		{
			conveyor_mg.move(0);
		}

		// mogo mech
		if (master.get_digital_new_press(DIGITAL_A))
		{
			mogo_active = !mogo_active;
			pros::delay(20);
		}

		if (mogo_active)
		{
			mogo_solenoid.set_value(true);
		}
		else
		{
			mogo_solenoid.set_value(false);
		}

		pros::delay(20);
	}
}
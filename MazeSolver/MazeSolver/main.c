#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define F_CPU 20000000  // system clock is 20 MHz
#include <util/delay.h> // uses F_CPU to achieve us and ms delays

#define TURN_DELAY_MS 100
#define INCH_DELAY_MS 100


/****************** MOTOR CODE ******************/
// Motor Initialization routine -- this function must be called
//  before you use any of the above functions
void motors_init()
{
	// configure for inverted PWM output on motor control pins:
	//  set OCxx on compare match, clear on timer overflow
	//  Timer0 and Timer2 count up from 0 to 255
	TCCR0A = TCCR2A = 0xF3;

	// use the system clock/8 (=2.5 MHz) as the timer clock
	TCCR0B = TCCR2B = 0x03;

	// initialize all PWMs to 0% duty cycle (braking)
	OCR0A = OCR0B = OCR2A = OCR2B = 0;

	// set PWM pins as digital outputs (the PWM signals will not
	// appear on the lines if they are digital inputs)
	DDRD |= (1 << PORTD3) | (1 << PORTD5) | (1 << PORTD6);
	DDRB |= (1 << PORTB3);
}

void set_motor_power(unsigned int power_left, unsigned int power_right)
{
	OCR0B = power_right;
	OCR2B = power_left;
}

void inch()
{
	set_motor_power(10,10);
	delay_ms(INCH_DELAY_MS);
	read_sensors();
	set_motor_power(0,0);

}

void turnLeft()
{
	// Make all output compare values 0
	set_motor_power(0,0);

	// Motor 1 reverse, motor 2 forward
	OCR0A = 10;
	OCR2B = 10;

	delay_ms(TURN_DELAY_MS);

	// Turn off both motors
	OCR0A = 0;
	OCR2B = 0;
}

void turnRight()
{
	// Make all output compare values 0
	set_motor_power(0,0);

	// Motor 1 forward, motor 2 reverse
	OCR0B = 10;
	OCR2A = 10;

	delay_ms(TURN_DELAY_MS);

	// Turn off both motors
	OCR0B = 0;
	OCR2A = 0;
}

/****************** End MOTOR CODE ******************/

/****************** SESNOR CODE ******************/

// delay for time_ms milliseconds by looping
//  time_ms is a two-byte value that can range from 0 - 65535
//  a value of 65535 (0xFF) produces an infinite delay
void delay_ms(unsigned int time_ms)
{
	// _delay_ms() comes from <util/delay.h> and can only
	//  delay for a max of around 13 ms when the system
	//  clock is 20 MHz, so we define our own longer delay
	//  routine based on _delay_ms()

	unsigned int i;

	for (i = 0; i < time_ms; i++)
	_delay_ms(1);
}

#define LD_THRESHOLD 0x500
#define TIMEOUT_THRESHOLD 0x1000

volatile int hasItBeenReadYet[5];
volatile bool lightDarkBits[5];
volatile char path[30];
volatile int pathIndex = 0;

volatile int previous = 0;
volatile int count0;

void read_sensors()
{

	lightDarkBits[5];
	for (int i = 0; i < 5; ++i)
	{
		lightDarkBits[i] = false;
	}

	DDRC = 0xFF;
	PORTC = 0x00;

	PORTC = 0x1F; //Charge
	_delay_us(10);

	PORTC = 0x20; //Turn on LEDs
	DDRC = 0x20;  //Stop charging

	_delay_us(400);

	int values = PINC;
	lightDarkBits[0] = ((int)(values & (1 << 0)) == 1 << 0);
	lightDarkBits[1] = ((int)(values & (1 << 1)) == 1 << 1);
	lightDarkBits[2] = ((int)(values & (1 << 2)) == 1 << 2);
	lightDarkBits[3] = ((int)(values & (1 << 3)) == 1 << 3);
	lightDarkBits[4] = ((int)(values & (1 << 4)) == 1 << 4);
}

/****************** END SESNOR CODE ******************/

/****************** LINE TRACKING CODE ******************/
void trackLine()
{

	int forward_motor_power = 25;
	const int max = 25; // Maximum value for PID controller

	int error = 0;  // Calculated error

	// Gain constants for PID controller
	float kP = 3;

	int motor_power_left = forward_motor_power;
	int motor_power_right = forward_motor_power;
	set_motor_power(forward_motor_power, forward_motor_power);

	//Calculate error
	if (lightDarkBits[0] == 1 && lightDarkBits[2] == 0 && lightDarkBits[3] == 0 && lightDarkBits[4] == 0)
	{
		if (lightDarkBits[1] == 1)
		{
			error = 4;
		}
		else
		{
			error = 5;
		}
	}
	if (lightDarkBits[1] == 1 && lightDarkBits[0] == 0 && lightDarkBits[3] == 0 && lightDarkBits[4] == 0)
	{
		if (lightDarkBits[2] == 1)
		{
			error = 1;
		}
		else
		{
			error = 2;
		}
	}
	if (lightDarkBits[2] == 1 && lightDarkBits[0] == 0 && lightDarkBits[1] == 0 && lightDarkBits[4] == 0)
	{
		if (lightDarkBits[3] == 1)
		{
			error = -1;
		}
		else
		{
			error = 0;
		}
	}
	if (lightDarkBits[3] == 1 && lightDarkBits[0] == 0 && lightDarkBits[1] == 0 && lightDarkBits[2] == 0)
	{
		if (lightDarkBits[4] == 1)
		{
			error = -4;
		}
		else
		{
			error = -2;
		}
	}
	if (lightDarkBits[4] == 1 && lightDarkBits[1] == 0 && lightDarkBits[2] == 0 && lightDarkBits[3] == 0 && lightDarkBits[0] == 0)
	{
		error = -5;
	}


	int feedback = kP * error;

	// Cap feedback at max value
	if (feedback > max)
	feedback = max;
	if (feedback < -max)
	feedback = -max;

	if (error == 0)
	{
		set_motor_power(forward_motor_power, forward_motor_power);
	}
	else
	{
		set_motor_power(forward_motor_power + feedback, forward_motor_power - feedback);
	}
}

// detectIntersection
// This function should be called when an intersection is detected
// Performs sensor reading & decision making logic to determine type of intersection,
// traverse the intersection, and log the decision made
void detectIntersection()
{
	// Stop robot
	set_motor_power(0,0);

	// Intersection is either left only or left & forward
	if(lightDarkBits[0] == 1  && lightDarkBits[4] == 0)
	{
		inch();
		if(lightDarkBits[2] == 0)
		{
			turnLeft();
			
		}
		else
		{
			//turnLeft();
			path[pathIndex++] = 'L';
			while(true){}
		}

	}
	// Intersection is either right only or right & forward
	else if(lightDarkBits[0] == 0 && lightDarkBits[4] == 1)
	{
		inch();
		if(lightDarkBits[2] == 0)
		{
			turnRight();
		}
		else
		{
			path[pathIndex++] = 'S';
			while(true){}
		}
	}
	// Intersection is either T or cross
	else
	{
		inch();
		//turnLeft();
		path[pathIndex++] = 'L';
		while(true){}
	}


}

void uturn()
{
	//turnLeft();
	//turnLeft();
	path[pathIndex++] = 'U';
	while(true){}
}



void finalEnd(){
	while(1){
		turnLeft();
	}
}

char simplifyPath(char first, char second){
	if(first == 'L'){
		if(second == 'S'){
			return 'R';
		}
		else if(second == 'L'){
			return 'S';
		}
		else if(second == 'R'){
			return 'U';
		}
	}
	else if(first == 'S'){
		if(second == 'L'){
			return 'R';
		}
		else if(second == 'R'){
			return 'L';
		}
	}
	else if(first == 'R' && second == 'L'){
		return 'U';
	}
}

void shiftPath(){

	for(int i = pathIndex - 1; i >=0; --i){
		if(path[i] = 'e'){
			for(int j = i; j < pathIndex; ++j){
				path[j] = path[j+1];
			}
			--pathIndex;
		}
		
	}
}

/****************** END LINE TRACKING CODE ******************/


int main()
{
	// Initialize motors
	motors_init();
	
	for (int i = 0; i < 30; ++i){//initialize path to empty
		path[i] = 'e';
	}

	// loop here forever to keep the program counter from
	//  running off the end of our program
	while (1)
	{
		// Read sensors should return the byte containing whether each sensor was high or low
		read_sensors();

		if (lightDarkBits[0] == 1 || lightDarkBits[4] == 1)
		{
			detectIntersection();
		}
		else if(lightDarkBits[0] == 0 && lightDarkBits[1] == 0 && lightDarkBits[2] == 0 && lightDarkBits[3] == 0 && lightDarkBits[4] == 0)
		{
			uturn();
		}
		else if (lightDarkBits[0] == 1 && lightDarkBits[1] == 1 && lightDarkBits[2] == 1 && lightDarkBits[3] == 1 && lightDarkBits[4] == 1){
			set_motor_power(0, 0);
			break;
		}
		else
		{
			trackLine();
		}
	}
		
		//Handle path simplification
		bool isDone = false;
		while(!isDone){
			isDone = true;
			for(int i = pathIndex - 1; i >= 0; --i){
				if(path[i] == 'U'){
					isDone = false;
					path[i] == simplifyPath(path[i -1], path[i + 1]);
					shiftPath();
				}
			}
		}
		
		
		
		//start bot again with pre-programmed path when sees mid 3 dark
		pathIndex = 0;
		while(!(lightDarkBits[0] == 0 && lightDarkBits[1] == 1 && lightDarkBits[2] == 1 && lightDarkBits[3] == 1 && lightDarkBits[4] == 0)){//placed at start
			
		}
		while (1)
		{
			// Read sensors should return the byte containing whether each sensor was high or low
			read_sensors();

			if (lightDarkBits[0] == 1 || lightDarkBits[4] == 1)
			{
				inch();
				if(path[pathIndex] == 'L'){
					turnLeft();
				}
				else if (path[pathIndex] == 'R'){
					turnRight();
				}
				else {
					trackLine();
				}
			}
			else if (lightDarkBits[0] == 1 && lightDarkBits[1] == 1 && lightDarkBits[2] == 1 && lightDarkBits[3] == 1 && lightDarkBits[4] == 1){
				finalEnd();
			}
			else
			{
				trackLine();
			}
		}
	return 0;
}

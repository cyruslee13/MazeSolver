#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>


volatile int test1;
volatile int test2;

void set_motor_power(unsigned int power_left, unsigned int power_right)
{
	OCR0B = power_right;
	OCR2B = power_left;
}


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

#define F_CPU 20000000  // system clock is 20 MHz
#include <util/delay.h>  // uses F_CPU to achieve us and ms delays

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


volatile int hasItBeenReadYet [5];
volatile bool lightDarkBits [5];
volatile int previous = 0;
//volatile bool previous0;
//volatile bool previous1;
//volatile bool previous2;
//volatile bool previous3;
//volatile bool previous4;
volatile int count0;
//volatile int count1;
//volatile int count2;
//volatile int count3;
//volatile int count4;

volatile int* read_sensors(){
	
	lightDarkBits[5];
	for (int i = 0; i < 5; ++i){
		lightDarkBits[i] = false;
	}
	
	DDRC = 0xFF;
	PORTC = 0x00;
	
	PORTC = 0x1F; //Charge
	_delay_us(10);
	
	PORTC = 0x20;//Turn on LEDs
	DDRC = 0x20;//Stop charging
	
	
	_delay_us(400);
	
	int values = PINC;
	lightDarkBits[0] = ((int)(values&(1<<0)) == 1<<0);
	lightDarkBits[1] = ((int)(values&(1<<1)) == 1<<1);
	lightDarkBits[2] = ((int)(values&(1<<2)) == 1<<2);
	lightDarkBits[3] = ((int)(values&(1<<3)) == 1<<3);
	lightDarkBits[4] = ((int)(values&(1<<4)) == 1<<4);
	
	////previous0 = 1;
	////previous1 = 1;
	////previous2 = 1;
	////previous3 = 1;
	////previous4 = 1;
	//previous = 0x3F;
	//TIMSK1 = 0x02;
	//PCMSK1 = 0x1F;
	//DDRC = 0xFF;
	//PORTC = 0x00;
	//TCCR1B = 0x00;
	//TCNT1 = 0x00;
	//TCCR1A = 0x40;
	//OCR1A = TIMEOUT_THRESHOLD;
	//
	////Charge sensors
	//PORTC = 0x1F;
	//_delay_us(10);
	//
	//PORTC = 0x20;//Turn on LEDs
	//DDRC = 0x20;//Stop charging
	//
	//hasItBeenReadYet[5];
	//for (int i = 0; i < 5; ++i){
	//hasItBeenReadYet[i] = 0;
	//}
	//lightDarkBits[5];
	//for (int i = 0; i < 5; ++i){
	//lightDarkBits[i] = 1;
	//}
	////previous0 = false;
	////previous1 = false;
	////previous2 = false;
	////previous3 = false;
	////previous4 = false;
	//
	//sei();
	//
	//bool isDone = false;
	//
	//while(!isDone){
	//isDone = true;
	//for(int i = 0; i < 5; ++i){
	//if(hasItBeenReadYet[i] == 0){
	//isDone = false;
	//}
	//}
	//}
	//return lightDarkBits;
	
}

ISR(PCINT1_vect){
	
	//bool isrPrevious0 = previous0;
	//bool isrPrevious1 = previous1;
	//bool isrPrevious2 = previous2;
	//bool isrPrevious3 = previous3;
	//bool isrPrevious4 = previous4;
	//
	//previous0 = PINC0;
	//previous1 = PINC1;
	//previous2 = PINC2;
	//previous3 = PINC3;
	//previous4 = PINC4;
	int isrPrevious = previous;
	previous = PINC;

	
	long timerVal = TCNT1;
	
	test1 = isrPrevious&(1<<0);
	test2 = previous&(1<<0);
	
	if((int)(isrPrevious&(1<<0)) != (int)(previous&(1<<0))){
		if(timerVal < LD_THRESHOLD){
			lightDarkBits[0] = 0;
		}
		hasItBeenReadYet[0] = 1;
		count0 = timerVal;
	}
	//
	//if(isrPrevious1 != previous1){
	//if(timerVal < LD_THRESHOLD){
	//lightDarkBits[1] = 0;
	//}
	//hasItBeenReadYet[1] = 1;
	//count1 = timerVal;
	//}
	//
	//if(isrPrevious2 != previous2){
	//if(timerVal < LD_THRESHOLD){
	//lightDarkBits[2] = 0;
	//}
	//hasItBeenReadYet[2] = 1;
	//count2 = timerVal;
	//}
	//
	//if(isrPrevious3 != previous3){
	//if(timerVal < LD_THRESHOLD){
	//lightDarkBits[3] = 0;
	//}
	//hasItBeenReadYet[3] = 1;
	//count3 = timerVal;
	//}
	//
	//if(isrPrevious4 != previous4){
	//if(timerVal < LD_THRESHOLD){
	//lightDarkBits[4] = 0;
	//}
	//hasItBeenReadYet[4] = 1;
	//count4 = timerVal;
	//}
	
}

ISR(TIMER1_COMPA_vect){
	for (int i = 0; i < 5; ++i){
		hasItBeenReadYet[i] = 1;
	}
}


int main()
{
	// Initialize motors
	motors_init();
	
	//while(1){
		int forward_motor_power = 25;
		
		int motor_power_left = forward_motor_power;
		int motor_power_right = forward_motor_power;
		set_motor_power(forward_motor_power, forward_motor_power);
	//}
	
	
	//Initialize sensors
	//sensors_init();
	
	//volatile int* sensorReadings;
	int error = 0; // Calculated error
	int last_error; // Error at last iteration
	
	const int max = 25; // Maximum value for PID controller
	
	// Gain constants for PID controller
	float kP = 3;
	float kI = 0;
	float kD = 0;
	
	// Variables to store calculated PID values
	int derivative;
	int integral = 0;
	
	// loop here forever to keep the program counter from
	//  running off the end of our program
	while(1)
	{
	// Read sensors should return the byte containing whether each sensor was high or low
	
	read_sensors();
	
	last_error = error;
	//Calculate error
	if(lightDarkBits[0] == 1 && lightDarkBits[2] == 0 && lightDarkBits[3] == 0 && lightDarkBits[4] == 0 ){
	if(lightDarkBits[1] == 1){
	error = 4;
	} else {
	error = 5;
	}
	}
	if(lightDarkBits[1] == 1 && lightDarkBits[0] == 0 && lightDarkBits[3] == 0 && lightDarkBits[4] == 0){
	if(lightDarkBits[2] == 1){
	error = 1;
	} else {
	error = 2;
	}
	}
	if (lightDarkBits[2] == 1 && lightDarkBits[0] == 0 && lightDarkBits[1] == 0 && lightDarkBits[4] == 0){
	if(lightDarkBits[3] == 1){
	error = -1;
	} else {
	error = 0;
	}
	}
	if (lightDarkBits[3] == 1 && lightDarkBits[0] == 0 && lightDarkBits[1] == 0 && lightDarkBits[2] == 0){
	if (lightDarkBits[4] == 1){
	error = -4;
	} else{
	error = -2;
	}
	}
	if(lightDarkBits[4] == 1 && lightDarkBits[1] == 0 && lightDarkBits[2] == 0 && lightDarkBits[3] == 0 && lightDarkBits[0] == 0) {
	error = -5;
	}
	
	
	
	int derivative = error - last_error;
	integral += error;
	
	int feedback = kP*error + kI*integral + kD*derivative;
	
	// Cap feedback at max value
	if(feedback > max)
	feedback = max;
	if(feedback < -max)
	feedback = -max;
	
	if(error == 0){
	set_motor_power(forward_motor_power, forward_motor_power);
	}else{
	set_motor_power(forward_motor_power+feedback, forward_motor_power-feedback);
	}
	
	}
	
	return 0;
}


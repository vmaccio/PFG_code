#include <stdio.h>    // Used for printf() statements
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <wiringPi.h> // Include WiringPi library!

#include <time.h>
	
void moverServoPrioritario(int pwm);

void moverServo(int pwm);

void liberarServo();

void init_servoOcupado();
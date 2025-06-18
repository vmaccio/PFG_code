#include <stdio.h>    // Used for printf() statements
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <softPwm.h>
#include <wiringPi.h> // Include WiringPi library!
#include <sys/time.h>
#include "devices.h"


//#include <softPwm.h>

#include <time.h>

static int servoOcupado = 0;

void moverServoPrioritario(int pwm){
	servoOcupado = 1;
	setServoPulse(pwm);
}

void moverServo(int pwm){
	if(servoOcupado == 0){
		setServoPulse(pwm);
	}
}

void liberarServo(){
	servoOcupado = 0;
}

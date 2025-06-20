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
#include <semaphore.h>


//#include <softPwm.h>

#include <time.h>

static int servoOcupado = 0;

static sem_t servoOcupado_sem;

void init_servoOcupado() {
	sem_init(&servoOcupado_sem, 0, 1);
}

void moverServoPrioritario(int pwm){
	sem_wait(&servoOcupado_sem);
	servoOcupado = 1;
	setServoPulse(pwm);
	sem_post(&servoOcupado_sem);
}

void moverServo(int pwm){
	sem_wait(&servoOcupado_sem);
	if(servoOcupado == 0){
		setServoPulse(pwm);
	}
	sem_post(&servoOcupado_sem);
}

void liberarServo(){
	sem_wait(&servoOcupado_sem);
	servoOcupado = 0;
	sem_post(&servoOcupado_sem);
}

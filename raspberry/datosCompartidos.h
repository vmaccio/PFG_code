#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <softPwm.h>
#include <wiringPi.h>
#include <math.h>
#include <time.h>
#include <arpa/inet.h>
#include "devices.h"
#include "servoController.h"
#include <pthread.h>
#include <semaphore.h>

/* // Declaración de variables globales
static int joystickx, joysticky, limite_cm_aviso, avisoObstaculo, infrarrojos, max_speed, flag_evasion;
static float distancia, totalGiroGrados, resta;
static double totalGiroVirtual, dist_v;

// Declaración de semáforos
static sem_t joystickx_sem, joysticky_sem, limite_cm_aviso_sem, avisoObstaculo_sem, infrarrojos_sem, max_speed_sem, flag_evasion_sem;
static sem_t distancia_sem, totalGiroGrados_sem, resta_sem, totalGiroVirtual_sem, dist_v_sem;
 */
// Funciones de inicialización
void init_semaforos_variables_compartidas(void);

// Getters y setters
int get_joystickx(void);
void set_joystickx(int value);

int get_joysticky(void);
void set_joysticky(int value);

int get_limite_cm_aviso(void);
void set_limite_cm_aviso(int value);

int get_avisoObstaculo(void);
void set_avisoObstaculo(int value);

int get_infrarrojos(void);
void set_infrarrojos(int value);

int get_max_speed(void);
void set_max_speed(int value);

int get_flag_evasion(void);
void set_flag_evasion(int value);

float get_distancia(void);
void set_distancia(float value);

float get_totalGiroGrados(void);
void set_totalGiroGrados(float value);

float get_resta(void);
void set_resta(float value);

double get_totalGiroVirtual(void);
void set_totalGiroVirtual(double value);

double get_dist_v(void);
void set_dist_v(double value);

void update_totalGiroGrados(float value);
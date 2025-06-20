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
#include "datosCompartidos.h"

static int joystickx, joysticky, limite_cm_aviso, avisoObstaculo, infrarrojos, max_speed, flag_evasion;
static float distancia, totalGiroGrados, resta
static double totalGiroVirtual, dist_v;

static sem_t joystickx_sem, joysticky_sem, limite_cm_aviso_sem, avisoObstaculo_sem, infrarrojos_sem, max_speed_sem, flag_evasion_sem;
static sem_t distancia_sem, totalGiroGrados_sem, resta_sem, totalGiroVirtual_sem, dist_v_sem;

void init_semaforos_variables_compartidas() {
    sem_init(&joystickx_sem, 0, 1);
    sem_init(&joysticky_sem, 0, 1);
    sem_init(&limite_cm_aviso_sem, 0, 1);
    sem_init(&avisoObstaculo_sem, 0, 1);
    sem_init(&infrarrojos_sem, 0, 1);
    sem_init(&max_speed_sem, 0, 1);
    sem_init(&flag_evasion_sem, 0, 1);
    sem_init(&distancia_sem, 0, 1);
    sem_init(&totalGiroGrados_sem, 0, 1);
    sem_init(&resta_sem, 0, 1);
    sem_init(&totalGiroVirtual_sem, 0, 1);
    sem_init(&dist_v_sem, 0, 1);
}

int get_joystickx() {
    int value;
    sem_wait(&joystickx_sem);
    value = joystickx;
    sem_post(&joystickx_sem);
    return value;
}

void set_joystickx(int value) {
    sem_wait(&joystickx_sem);
    joystickx = value;
    sem_post(&joystickx_sem);
}

int get_joysticky() {
    int value;
    sem_wait(&joysticky_sem);
    value = joysticky;
    sem_post(&joysticky_sem);
    return value;
}

void set_joysticky(int value) {
    sem_wait(&joysticky_sem);
    joysticky = value;
    sem_post(&joysticky_sem);
}

int get_limite_cm_aviso() {
    int value;
    sem_wait(&limite_cm_aviso_sem);
    value = limite_cm_aviso;
    sem_post(&limite_cm_aviso_sem);
    return value;
}

void set_limite_cm_aviso(int value) {
    sem_wait(&limite_cm_aviso_sem);
    limite_cm_aviso = value;
    sem_post(&limite_cm_aviso_sem);
}

int get_avisoObstaculo() {
    int value;
    sem_wait(&avisoObstaculo_sem);
    value = avisoObstaculo;
    sem_post(&avisoObstaculo_sem);
    return value;
}

void set_avisoObstaculo(int value) {
    sem_wait(&avisoObstaculo_sem);
    avisoObstaculo = value;
    sem_post(&avisoObstaculo_sem);
}

int get_infrarrojos() {
    int value;
    sem_wait(&infrarrojos_sem);
    value = infrarrojos;
    sem_post(&infrarrojos_sem);
    return value;
}

void set_infrarrojos(int value) {
    sem_wait(&infrarrojos_sem);
    infrarrojos = value;
    sem_post(&infrarrojos_sem);
}

int get_max_speed() {
    int value;
    sem_wait(&max_speed_sem);
    value = max_speed;
    sem_post(&max_speed_sem);
    return value;
}

void set_max_speed(int value) {
    sem_wait(&max_speed_sem);
    max_speed = value;
    sem_post(&max_speed_sem);
}

int get_flag_evasion() {
    int value;
    sem_wait(&flag_evasion_sem);
    value = flag_evasion;
    sem_post(&flag_evasion_sem);
    return value;
}

void set_flag_evasion(int value) {
    sem_wait(&flag_evasion_sem);
    flag_evasion = value;
    sem_post(&flag_evasion_sem);
}

float get_distancia() {
    float value;
    sem_wait(&distancia_sem);
    value = distancia;
    sem_post(&distancia_sem);
    return value;
}

void set_distancia(float value) {
    sem_wait(&distancia_sem);
    distancia = value;
    sem_post(&distancia_sem);
}

float get_totalGiroGrados() {
    float value;
    sem_wait(&totalGiroGrados_sem);
    value = totalGiroGrados;
    sem_post(&totalGiroGrados_sem);
    return value;
}

void set_totalGiroGrados(float value) {
    sem_wait(&totalGiroGrados_sem);
    totalGiroGrados = value;
    sem_post(&totalGiroGrados_sem);
}

float get_resta() {
    float value;
    sem_wait(&resta_sem);
    value = resta;
    sem_post(&resta_sem);
    return value;
}

void set_resta(float value) {
    sem_wait(&resta_sem);
    resta = value;
    sem_post(&resta_sem);
}

double get_totalGiroVirtual() {
    double value;
    sem_wait(&totalGiroVirtual_sem);
    value = totalGiroVirtual;
    sem_post(&totalGiroVirtual_sem);
    return value;
}

void set_totalGiroVirtual(double value) {
    sem_wait(&totalGiroVirtual_sem);
    totalGiroVirtual = value;
    sem_post(&totalGiroVirtual_sem);
}

double get_dist_v() {
    double value;
    sem_wait(&dist_v_sem);
    value = dist_v;
    sem_post(&dist_v_sem);
    return value;
}

void set_dist_v(double value) {
    sem_wait(&dist_v_sem);
    dist_v = value;
    sem_post(&dist_v_sem);
}

void update_totalGiroGrados(float value) {
    sem_wait(&totalGiroGrados_sem);
    totalGiroGrados += value;
    sem_post(&totalGiroGrados_sem);
}
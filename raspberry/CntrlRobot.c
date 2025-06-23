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


//Prueba git
//Inicializo las variables que se van a usar a lo largo de la ejecucion
#define NUM_THREADS 4
#define MAX_SPEED_CLOCKWISE 130
#define MAX_SPEED_COUNTER_CLOCKWISE 170

    int sock = 0, valread, i, boton, devices;
	//int joystickx, joysticky, infrarrojos;
	//int analog_sensors[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	struct sockaddr_in serv_addr;
	//float distancia = 0.0;
	//double dist_v = '0';
	char buffer[1024] = {0};
	char combined_data[256] = {0};
	char temp_buffer[32];
	int dist_alerta = 1024;
	//int flag_evasion = 0;
	//int limite_cm_aviso = 5;
	//int max_speed;
	//float totalGiroGrados;
	//double totalGiroVirtual;
	//int avisoObstaculo = 0;
	//float resta = 0;
	


//Este hilo gestiona la proximidad de los objetos y actua para esquivarlos
void *proximidad (){
	while (1)
	{
		//Leo la distancia del ultrasonidos (cm)
		set_distancia(getDistance());
		//Leo el potenciometro deslizante (0-1023)
		dist_alerta = read_single_sensor(3);
		dist_alerta = dist_alerta / 256;
		//Obtengo 4 rangos de distancia de deteccion
		switch (dist_alerta)
		{
		case 0:
			set_limite_cm_aviso(10);
			break;
		case 1:
			set_limite_cm_aviso(20);
			break;
		case 2:
			set_limite_cm_aviso(30);
			break;
		case 3:
			set_limite_cm_aviso(40);
			break;		
		default:
			set_limite_cm_aviso(20);
			break;
		}

		//Si alguno de los sensores de distanicia lee por debajo del limite
		//Giro el robot para evitarlo y marco el flag a 1 para que le virtual tambien gire
		if (get_distancia() <= get_limite_cm_aviso() || get_dist_v() <= get_limite_cm_aviso()){
			set_flag_evasion(1);
			moverServoPrioritario(0);
			
			while (get_distancia() <= get_limite_cm_aviso() || get_dist_v() <= get_limite_cm_aviso())
			{
				moverServoPrioritario(170);
				set_distancia(getDistance());
			}
			set_flag_evasion(0);
			moverServoPrioritario(0);
			liberarServo();
		}
		usleep(1250);
	}
	pthread_exit (NULL);
}

//En este hilo se gestiona el movimiento del robot mediante el Joystick
void *movimiento (){
	while (1)
	{
		//Leo el Joystick, el potenciometro giratorio y el sensor infrarojo
		set_joystickx(read_single_sensor(1));
		set_joysticky(read_single_sensor(4));
		
		set_max_speed(read_single_sensor(2)/1024);
		set_infrarrojos(read_infrared());
		
		switch (get_infrarrojos()){//si esta en 1 el Josytick controla el robot
		case 1: 
			//marco 400-600 como zona muerta para evitar giros no deseados
			if(get_joysticky() >= 400 && get_joysticky() <= 600){
				if(get_joystickx() < 400){
					//atras
					set_led_1(1);
				}else if (get_joystickx() > 600)
				{
					//delante
					set_led_2(1);
				}else if (get_joystickx() >= 400 && get_joystickx() <= 600)
				{
					//quieto
					set_led_1(0);
					set_led_2(0);
				}
				moverServo(151);
			}else if (get_joysticky() < 200)
			{
				//giro izq
				moverServo(130);
			}
			else if (get_joysticky() < 400)
			{
				//giro izq
				moverServo(145);
			}
			else if (get_joysticky() > 800)
			{
				//giro drch
				moverServo(170);
			}
			else if (get_joysticky() > 600)
			{
				//giro drch
				moverServo(155);
			}
			break;

		case 0: //Si es 0 mantengo rumbo excepto que se encuentre un obstaculo
			
			break;
		}
		usleep(2500);
	}
	pthread_exit (NULL);
}	 

void *info (){
	while(1){
		system("clear");
		printf("Distancia a objeto: %.2fcm \t Limite de deteccion: %dcm\n", get_distancia(), get_limite_cm_aviso());
		printf("Alerta de obstaculo: %d\n", get_flag_evasion());
		printf("Joystick X: %d \t Joystick Y: %d\n", get_joystickx(), get_joysticky());
		printf("Infrarojos: %d\n", get_infrarrojos());
		printf("Posicion: %.2fº\n", get_totalGiroGrados());
		printf("Posicion virtual: %.2fº\t Desviacion: %.2f\n", get_totalGiroVirtual(), get_resta());
		printf("Distancia a objeto virtual: %.2fcm\n", get_dist_v());
		
		usleep(250000);
	}
	pthread_exit (NULL);
}

void *calibracion (){
	while (1)
	{
		update_totalGiroGrados(leerGiroscopioX360());
		set_resta(abs(get_totalGiroGrados() - get_totalGiroVirtual()));
		//es el unico tan bajo porque sin no funcionaba bien
		usleep(100);
	}
	pthread_exit (NULL);
}

void *conexion (){
	sleep(1);
	while (1)
	{	 
		//Concatenamos en un string todos los datos a enviar
		snprintf(temp_buffer, sizeof(temp_buffer), "%d ", get_infrarrojos());
		strcat(combined_data, temp_buffer);
		snprintf(temp_buffer, sizeof(temp_buffer), "%d ", get_max_speed());
		strcat(combined_data, temp_buffer);
		snprintf(temp_buffer, sizeof(temp_buffer), "%d ", get_joystickx());
		strcat(combined_data, temp_buffer);
		snprintf(temp_buffer, sizeof(temp_buffer), "%d ", get_joysticky());
		strcat(combined_data, temp_buffer);
		snprintf(temp_buffer, sizeof(temp_buffer), "%d ", get_flag_evasion());
		strcat(combined_data, temp_buffer);
		snprintf(temp_buffer, sizeof(temp_buffer), "%f", get_totalGiroGrados());
		strcat(combined_data, temp_buffer);
		// Mandamos el array por socket
		send(sock, combined_data, strlen(combined_data), 0);
		// Leemos la respuesta de socket
		valread = read(sock, buffer, 1024);
		if(valread > 0){
			buffer[valread] = '\0';
		}
		// Limpiamos el array combined_data para el próximo envío
		combined_data[0] = '\0';
		//Creamos variables temporales y un token para recorrer la respuesta
		double temp_dist_v = 0, temp_giro_v = 0;
		//Separacion del string recibido en tokens con un espacio como limitador
        char *token = strtok(buffer, " ");
        if (token != NULL) {
			//Conversion del primer token a float y almacenamiento del valor
            temp_dist_v = atof(token);
            //Se extaer el siguiente token
            token = strtok(NULL, " ");
            //Si el token no esta vacio significa que se ha extarido el dato
            if (token != NULL) {
				//Conversion del segundo valor
                temp_giro_v = atof(token);
                set_dist_v(temp_dist_v);
                set_totalGiroVirtual(temp_giro_v);
            } else {//En caso de error se imprime por pantalla
                printf("Segundo valor no recibido: %s\n", buffer);
            }
        } else {
            printf("Buffer vacío o incorrecto: %s\n", buffer);
        }
		//Vacio el buffer para el proximo envio
		memset(buffer, 0, sizeof(buffer));
		usleep(1250);
	}
	
	pthread_exit (NULL);
}

int main(void)
{
	pthread_t thread [NUM_THREADS];

	// Crear socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error al crear el socket \n");
		return -1;
	}

	// conectar con el socket de webots, importante poner la dirección ip del ordenador destino
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000);
	//CAMBIAR IP
	if (inet_pton(AF_INET, "192.168.1.183", &serv_addr.sin_addr) <= 0)
	{
		printf("\n Dirección no válida \n");
		return -1;
	}

	// Conectar al servidor
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Falló la conexión \n");
		return -1;
	}

	// Inicializamos la libreria datosCompartidos
	init_semaforos_variables_compartidas();
	init_servoOcupado();

	// Iniciamos los devices con todos los pines
	printf("Testing devices ... \n");

	devices = init_devices();
	printf("Devices initialized: %d \n", devices);

	printf("\nEntra en calibracion\n");

	delay(2000);
	i = 0;
	
	moverServo(151);
	delay(1000);
	
	printf("Coloque el servomotor de forma recta alineado con el robot moviendo el Joystick.\n");
	printf("Cuando este colocado pulse el boton durante 3 segundos.\n");


	//Se usa el joystick para colocar el motor en la posicion inicial
	//Una vez se este satisfecho, se pulsa el boton durante 3 segundos y inicia la ejecucion
	while (i < 3) 
	{
		//1 si se esta pulsando el boton
		boton = read_button();
		if (boton == 1)
		{
			i = 0;
			set_joystickx(read_single_sensor(1));
			if(get_joystickx() < 300){
				moverServo(140);
			}else if (get_joystickx() > 700){
				moverServo(150);
			}else{
				moverServo(151);
			}
			delay(100);
		}
		else
		{
			i++;
			moverServo(0);
			delay(1000);
			printf("Confirmacion: %d \n", 4-i);
		}
	}
	
	//Creacion de todos los hilos

	pthread_create(&thread[0], NULL, proximidad, NULL);
	pthread_create(&thread[1], NULL, movimiento, NULL);
    pthread_create(&thread[2], NULL, info, NULL);
    pthread_create(&thread[3], NULL, calibracion, NULL);
    pthread_create(&thread[4], NULL, conexion, NULL);

	pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);
    pthread_join(thread[2], NULL);
    pthread_join(thread[3], NULL);
	pthread_join(thread[4], NULL);

	return 0;
}


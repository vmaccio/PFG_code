from controller import Robot, Motor, Accelerometer, Gyro
import socket
import select
import time
import math

# tiempo en [ms] de un paso de simulación
TIME_STEP = 64
MAX_SPEED = 3.5
MAX_TORQUE = 4
temp = 0

#Dimensiones del robot
STEPS_PER_REVOLUTION = 20
WHEEL_RADIUS = 0.0197 #en metros
AXLE_LENGTH = 0.056

#Calculos de giro
WHEEL_1_SPIN_DISTANCE = WHEEL_RADIUS * 2 * math.pi
AXLE_FULL_TURN_DISTANCE = AXLE_LENGTH * 2 * math.pi
TURN_FACTOR = AXLE_FULL_TURN_DISTANCE / WHEEL_1_SPIN_DISTANCE
LSB_SENSITIVITY = 131

# Crea la instancia del robot
robot = Robot()

# Inicializar sensores de proximidad
# Inicializamos todos pero solo usaremos ps0 y ps7 que son los dos delanteros
# ps = []
# psNames = ['ps0', 'ps1', 'ps2', 'ps3', 'ps4', 'ps5', 'ps6', 'ps7']
# for i in range(8):
#     ps.append(robot.getDevice(psNames[i]))
#     ps[i].enable(TIME_STEP)
    
# print(f"ps =  {ps}")

#prueba sensor distancia
long_range_sensor = robot.getDevice("tof")
long_range_sensor.enable(TIME_STEP)


# Inicializar los motores
left_motor = robot.getDevice("left wheel motor")
right_motor = robot.getDevice("right wheel motor")

gyro = robot.getDevice("gyro")
gyro.enable(1)
angulo_total = 0

left_motor.setPosition(float('inf')) #pone el motor a infinito para que siempre gire
right_motor.setPosition(float('inf'))
left_motor.setVelocity(0.0) #deja la velocidad a 0 para que no avance
right_motor.setVelocity(0.0)

# Configuración del servidor (Webots actúa como servidor)
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setblocking(False)  # Sockets no bloqueantes
server_address = ('0.0.0.0', 5000)
server_socket.bind(server_address)
server_socket.listen(1)
print("Esperando conexión de la Raspberry Pi...")

# Esperar la conexión sin bloquear el ciclo de simulación
client_socket = None
connected = False

def make_angle_tracker():
    last_time = robot.getTime()

    def get_degrees(angular_velocity_deg_per_sec):
        nonlocal last_time
        current_time = robot.getTime()
        elapsed = current_time - last_time
        last_time = current_time
        return angular_velocity_deg_per_sec * elapsed

    return get_degrees

tracker = make_angle_tracker()
left_speed = 0
right_speed = 0

while robot.step(TIME_STEP) != -1:
    if not connected:
        # Ver si hay una nueva conexión pendiente
        readable, _, _ = select.select([server_socket], [], [], 0)
        if readable:
            client_socket, client_address = server_socket.accept()
            client_socket.setblocking(False)  # Socket no bloqueante para el cliente
            connected = True
            print(f"Conexión establecida con {client_address}")

    if connected:
        readable, _, _ = select.select([client_socket], [], [], 0) #hace falta chequear en cada iteracion el socket o se puede hacer una unica vez???
        if readable:
            # Procesar los datos recibidos
            data = client_socket.recv(1024).decode('utf-8')
            data = data.split()
            if data:
                #print(f"Datos recibidos: {data}")
                infrarrojos = int(data[0])
                #print(infrarrojos)
                max_speed = int(data[1])
                #print(max_speed)
                joystickx = int(data[2])
                #print(joystickx)
                joysticky = int(data[3])
                #print(joysticky)
                flag_evasion = int(data[4])
                #print(flag_evasion)
                giroFisico = float(data[5])
                #print(giroFisico)

                #Sensor Larga distancia
                #Resto 20 porque el robot chocando contra la pared da valor 20
                distanciaLarga = long_range_sensor.getValue() - 20
                print("Long-range sensor value:", distanciaLarga)

                gyroZ = gyro.getValues()[2] / LSB_SENSITIVITY
                angulo_total = angulo_total + tracker(gyroZ)
                #print(angulo_total)

                # Asignamos a los valores el peligro de proximidad
                # Uso los dos de delante (0 y 7) como referencia de objetos delante
                #distancia_objeto_virtual = min(psValues[0], psValues[7])
                #print(distancia_objeto_virtual)

                if flag_evasion == 1: #se ha detectado obstaluclo en mundo real
                    left_speed = MAX_SPEED
                    right_speed = -MAX_SPEED
                else: #NO se ha detectado obstaculo real
                    if infrarrojos == 1:
                        if joysticky >= 400 and joysticky <= 600:
                            if joystickx < 200:
                                left_speed = -MAX_SPEED
                                right_speed = -MAX_SPEED
                            elif joystickx < 400:
                                left_speed = -MAX_SPEED/3
                                right_speed = -MAX_SPEED/3
                            elif joystickx > 800:
                                left_speed = MAX_SPEED
                                right_speed = MAX_SPEED
                            elif joystickx > 600:
                                left_speed = MAX_SPEED/3
                                right_speed = MAX_SPEED/3
                            else:
                                diferenciaGiro = giroFisico - angulo_total
                                #print(giroFisico, " - ",  angulo_total,  " = ", diferenciaGiro)
                                if (diferenciaGiro > 3):
                                    left_speed = -MAX_SPEED/2
                                    right_speed = MAX_SPEED/2
                                elif (diferenciaGiro < -3):
                                    left_speed = MAX_SPEED/2
                                    right_speed = -MAX_SPEED/2
                                else:
                                    left_speed = 0
                                    right_speed = 0
                        elif joysticky < 200:
                            left_speed = MAX_SPEED
                            right_speed = -MAX_SPEED
                        elif joysticky < 400:
                            left_speed = MAX_SPEED/3
                            right_speed = -MAX_SPEED/3
                        elif joysticky > 800:
                            left_speed = -MAX_SPEED
                            right_speed = MAX_SPEED
                        elif joysticky > 600:
                            left_speed = -MAX_SPEED/3
                            right_speed = MAX_SPEED/3

                left_motor.setVelocity(left_speed)
                right_motor.setVelocity(right_speed)


                # Responder a la Raspberry Pi 
                response = str(distanciaLarga) + " " + str(angulo_total)
                #print(response)
                client_socket.sendall(response.encode('utf-8'))
                response = " ";

C:: 
	gcc -c devices.c;
	gcc -c servoController.c
	gcc -c datosCompartidos.c
	gcc -c CntrlRobot.c -pthread; 
	gcc -o CntrlRobot CntrlRobot.o devices.o servoController.o datosCompartidos.o -lrt -lwiringPi -lm -pthread


clean::
	rm *.o CntrlRobot 

all:: C 

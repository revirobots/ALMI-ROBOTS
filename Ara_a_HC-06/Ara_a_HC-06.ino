#include <Servo.h>
#include <SoftwareSerial.h>

// === Configuración del módulo Bluetooth HC-05 ===
#define BT_RX_PIN 9   // RX del HC-05 conectado al pin 9 del Arduino
#define BT_TX_PIN 11  // TX del HC-05 conectado al pin 11 del Arduino
SoftwareSerial myBT(BT_RX_PIN, BT_TX_PIN); // RX, TX

// Definición de pines para los servos
const int FRONT_LEFT_V_PIN = 6;
const int FRONT_RIGHT_V_PIN = 3;
const int BACK_LEFT_V_PIN = 9;
const int BACK_RIGHT_V_PIN = 5;

const int FRONT_LEFT_H_PIN = 7;
const int FRONT_RIGHT_H_PIN = 8;
const int BACK_LEFT_H_PIN = 2;
const int BACK_RIGHT_H_PIN = 4;

// Crear objetos Servo
Servo servos[8]; // Array para todos los servos

// Índices para el array de servos
enum {
  FL_V = 0, FR_V = 1, BL_V = 2, BR_V = 3,
  FL_H = 4, FR_H = 5, BL_H = 6, BR_H = 7
};

// Configuración de ángulos
const int NEUTRAL = 90;       
const int FORWARD = 110;      
const int BACKWARD = 70;      
const int LIFT_HEIGHT = 30;   
const int STEP_ANGLE = 40;    

// Configuración de tiempos
const int TRANSITION_TIME = 100; 
const int MIN_DELAY = 15;       


// Variables para control de movimiento
int currentPositions[8];      
unsigned long lastMoveTime;  

// Estructura para definir un movimiento
struct Movement {
  int servoIndex;
  int targetAngle;
  int steps;
};

void setup() {
  Serial.begin(9600); // Para depuración
  myBT.begin(9600);   // Comunicación Bluetooth

  Serial.println("Robot Araña Cuadrúpedo listo para recibir comandos");

  // Inicializar y adjuntar todos los servos
  servos[FL_V].attach(FRONT_LEFT_V_PIN);
  servos[FR_V].attach(FRONT_RIGHT_V_PIN);
  servos[BL_V].attach(BACK_LEFT_V_PIN);
  servos[BR_V].attach(BACK_RIGHT_V_PIN);
  servos[FL_H].attach(FRONT_LEFT_H_PIN);
  servos[FR_H].attach(FRONT_RIGHT_H_PIN);
  servos[BL_H].attach(BACK_LEFT_H_PIN);
  servos[BR_H].attach(BACK_RIGHT_H_PIN);

  // Inicializar posiciones actuales
  for (int i = 0; i < 8; i++) {
    currentPositions[i] = NEUTRAL;
    servos[i].write(NEUTRAL);
  }

  delay(500); // Estabilización
}

void loop() {
  handleBluetoothCommands(); // Procesar comandos Bluetooth
}

void handleBluetoothCommands() {
  if (myBT.available() > 0) {
    char receivedData = myBT.read();

    // Descartar carácteres no imprimibles como \n o \r
    if (receivedData == '\n' || receivedData == '\r') {
      return;
    }

    Serial.print("Comando recibido: ");
    Serial.println(receivedData);

    switch (receivedData) {
      case 'F': // Adelante
        walkForwardDiagonal();
        break;
      case 'L': // Izquierda
        turnLeft();
        break;
      case 'R': // Derecha
        turnRight();
        break;
      case 'C': // Correr Original
        correrOriginal();
        break;
      case 'S': // Stop / Posición neutra
        restPosition();
        break;
      default:
        Serial.println("Comando no reconocido");
        break;
    }
  }
}

// Función para mover un servo gradualmente
void moveServoSmooth(int servoIndex, int targetAngle, int steps) {
  int startAngle = currentPositions[servoIndex];
  int angleChange = targetAngle - startAngle;
  if (angleChange == 0) return;
  for (int i = 1; i <= steps; i++) {
    int newAngle = startAngle + (angleChange * i / steps);
    servos[servoIndex].write(newAngle);
    currentPositions[servoIndex] = newAngle;
    delay(TRANSITION_TIME / steps);
  }
}

// Función para mover múltiples servos simultáneamente
void moveServosSimultaneously(Movement movements[], int count) {
  int maxSteps = 0;
  for (int i = 0; i < count; i++) {
    if (movements[i].steps > maxSteps) {
      maxSteps = movements[i].steps;
    }
  }

  int startAngles[count];
  int angleChanges[count];
  for (int i = 0; i < count; i++) {
    startAngles[i] = currentPositions[movements[i].servoIndex];
    angleChanges[i] = movements[i].targetAngle - startAngles[i];
  }

  for (int step = 1; step <= maxSteps; step++) {
    for (int i = 0; i < count; i++) {
      if (angleChanges[i] != 0) {
        int stepsForThisServo = movements[i].steps;
        if (step <= stepsForThisServo) {
          int newAngle = startAngles[i] + (angleChanges[i] * step / stepsForThisServo);
          servos[movements[i].servoIndex].write(newAngle);
          currentPositions[movements[i].servoIndex] = newAngle;
        }
      }
    }
    delay(MIN_DELAY);
  }
}

// Función para levantar y mover una pata
void liftAndMoveLeg(int verticalServo, int horizontalServo, int horizontalTarget) {
  Movement liftMove[] = {{verticalServo, NEUTRAL - LIFT_HEIGHT, 5}};
  Movement horizontalMove[] = {{horizontalServo, horizontalTarget, 5}};
  Movement lowerMove[] = {{verticalServo, NEUTRAL, 5}};

  moveServosSimultaneously(liftMove, 1);
  moveServosSimultaneously(horizontalMove, 1);
  moveServosSimultaneously(lowerMove, 1);
}

// Función para caminar hacia adelante usando marcha diagonal
void walkForwardDiagonal() {
  Movement liftDiagonal1[] = {{FL_V, NEUTRAL - LIFT_HEIGHT, 3}, {BR_V, NEUTRAL - LIFT_HEIGHT, 3}};
  Movement moveDiagonal1Forward[] = {{FL_H, FORWARD, 4}, {BR_H, FORWARD, 4}};
  Movement lowerDiagonal1[] = {{FL_V, NEUTRAL, 3}, {BR_V, NEUTRAL, 3}};
  Movement moveOthers1Backward[] = {{FR_H, BACKWARD, 4}, {BL_H, BACKWARD, 4}};

  Movement liftDiagonal2[] = {{FR_V, NEUTRAL - LIFT_HEIGHT, 3}, {BL_V, NEUTRAL - LIFT_HEIGHT, 3}};
  Movement moveDiagonal2Forward[] = {{FR_H, FORWARD, 4}, {BL_H, FORWARD, 4}};
  Movement lowerDiagonal2[] = {{FR_V, NEUTRAL, 3}, {BL_V, NEUTRAL, 3}};
  Movement moveOthers2Backward[] = {{FL_H, BACKWARD, 4}, {BR_H, BACKWARD, 4}};

  moveServosSimultaneously(liftDiagonal1, 2);
  moveServosSimultaneously(moveDiagonal1Forward, 2);
  moveServosSimultaneously(lowerDiagonal1, 2);
  moveServosSimultaneously(moveOthers1Backward, 2);
  moveServosSimultaneously(liftDiagonal2, 2);
  moveServosSimultaneously(moveDiagonal2Forward, 2);
  moveServosSimultaneously(lowerDiagonal2, 2);
  moveServosSimultaneously(moveOthers2Backward, 2);
}

// Función para girar a la derecha
void turnRight() {
  Movement liftDiagonal1[] = {{FL_V, NEUTRAL - LIFT_HEIGHT, 3}, {BR_V, NEUTRAL - LIFT_HEIGHT, 3}};
  Movement moveDiagonal1Turn[] = {{FL_H, FORWARD, 4}, {BR_H, BACKWARD, 4}};
  Movement lowerDiagonal1[] = {{FL_V, NEUTRAL, 3}, {BR_V, NEUTRAL, 3}};

  Movement liftDiagonal2[] = {{FR_V, NEUTRAL - LIFT_HEIGHT, 3}, {BL_V, NEUTRAL - LIFT_HEIGHT, 3}};
  Movement moveDiagonal2Turn[] = {{FR_H, BACKWARD, 4}, {BL_H, FORWARD, 4}};
  Movement lowerDiagonal2[] = {{FR_V, NEUTRAL, 3}, {BL_V, NEUTRAL, 3}};

  moveServosSimultaneously(liftDiagonal1, 2);
  moveServosSimultaneously(moveDiagonal1Turn, 2);
  moveServosSimultaneously(lowerDiagonal1, 2);
  moveServosSimultaneously(liftDiagonal2, 2);
  moveServosSimultaneously(moveDiagonal2Turn, 2);
  moveServosSimultaneously(lowerDiagonal2, 2);
}

// Función para girar a la izquierda
void turnLeft() {
  Movement liftDiagonal1[] = {{FL_V, NEUTRAL - LIFT_HEIGHT, 3}, {BR_V, NEUTRAL - LIFT_HEIGHT, 3}};
  Movement moveDiagonal1Turn[] = {{FL_H, BACKWARD, 4}, {BR_H, FORWARD, 4}};
  Movement lowerDiagonal1[] = {{FL_V, NEUTRAL, 3}, {BR_V, NEUTRAL, 3}};

  Movement liftDiagonal2[] = {{FR_V, NEUTRAL - LIFT_HEIGHT, 3}, {BL_V, NEUTRAL - LIFT_HEIGHT, 3}};
  Movement moveDiagonal2Turn[] = {{FR_H, FORWARD, 4}, {BL_H, BACKWARD, 4}};
  Movement lowerDiagonal2[] = {{FR_V, NEUTRAL, 3}, {BL_V, NEUTRAL, 3}};

  moveServosSimultaneously(liftDiagonal1, 2);
  moveServosSimultaneously(moveDiagonal1Turn, 2);
  moveServosSimultaneously(lowerDiagonal1, 2);
  moveServosSimultaneously(liftDiagonal2, 2);
  moveServosSimultaneously(moveDiagonal2Turn, 2);
  moveServosSimultaneously(lowerDiagonal2, 2);
}

// Función para posición de descanso
void restPosition() {
  Movement allToNeutral[] = {
    {FL_V, NEUTRAL, 5}, {FR_V, NEUTRAL, 5},
    {BL_V, NEUTRAL, 5}, {BR_V, NEUTRAL, 5},
    {FL_H, NEUTRAL, 5}, {FR_H, NEUTRAL, 5},
    {BL_H, NEUTRAL, 5}, {BR_H, NEUTRAL, 5}
  };
  moveServosSimultaneously(allToNeutral, 8);
}

// === Movimiento Correr Original (basado en walkForwardDiagonal) ===
void correrOriginal(int ciclos = 1) {
  Serial.println("Ejecutando movimiento 'Correr Original'");
  for (int i = 0; i < ciclos; i++) {
    Movement liftDiagonal1[] = {{FL_V, NEUTRAL - LIFT_HEIGHT, 3}, {BR_V, NEUTRAL - LIFT_HEIGHT, 3}};
    Movement moveDiagonal1Forward[] = {{FL_H, FORWARD, 4}, {BR_H, FORWARD, 4}};
    Movement lowerDiagonal1[] = {{FL_V, NEUTRAL, 3}, {BR_V, NEUTRAL, 3}};
    Movement moveOthers1Backward[] = {{FR_H, BACKWARD, 4}, {BL_H, BACKWARD, 4}};

    Movement liftDiagonal2[] = {{FR_V, NEUTRAL - LIFT_HEIGHT, 3}, {BL_V, NEUTRAL - LIFT_HEIGHT, 3}};
    Movement moveDiagonal2Forward[] = {{FR_H, FORWARD, 4}, {BL_H, FORWARD, 4}};
    Movement lowerDiagonal2[] = {{FR_V, NEUTRAL, 3}, {BL_V, NEUTRAL, 3}};
    Movement moveOthers2Backward[] = {{FL_H, BACKWARD, 4}, {BR_H, BACKWARD, 4}};

    moveServosSimultaneously(liftDiagonal1, 2);
    moveServosSimultaneously(moveDiagonal1Forward, 2);
    moveServosSimultaneously(lowerDiagonal1, 2);
    moveServosSimultaneously(moveOthers1Backward, 2);
    moveServosSimultaneously(liftDiagonal2, 2);
    moveServosSimultaneously(moveDiagonal2Forward, 2);
    moveServosSimultaneously(lowerDiagonal2, 2);
    moveServosSimultaneously(moveOthers2Backward, 2);
  }
}

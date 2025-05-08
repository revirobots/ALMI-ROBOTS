#include <Servo.h>
#include <SoftwareSerial.h>

// === Configuración del módulo Bluetooth HC-06 ===
#define BT_RX_PIN 9   // Pin 9 del Arduino -> TX del HC-06
#define BT_TX_PIN 11  // Pin 11 del Arduino -> RX del HC-06
SoftwareSerial myBT(BT_RX_PIN, BT_TX_PIN); // RX, TX

// --- Definiciones de Pines ---
#define PIN_RUEDA_DERECHA     13
#define PIN_RUEDA_IZQUIERDA   6
#define PIN_BRAZO_DERECHO     8
#define PIN_BRAZO_IZQUIERDO   10
#define PIN_CABEZA            12

// --- Objetos Servo ---
Servo servoRuedaDerecha;
Servo servoRuedaIzquierda;
Servo servoBrazoDerecho;
Servo servoBrazoIzquierdo;
Servo servoCabeza;

// --- Variables para movimiento continuo de brazos ---
static int anguloBrazos = 0;          // Ángulo inicial de los brazos (0 = abajo)
static int direccionBrazos = 1;       // 1 para subir, -1 para bajar
const int VELOCIDAD_BRAZOS_INCREMENTO = 1; 
const int BRAZO_ANGULO_MIN = 0;
const int BRAZO_ANGULO_MAX = 180;

// --- Variables para movimiento continuo de cabeza ---
static int anguloCabeza = 90;         // Centro
static int direccionCabeza = 1;       // 1 para derecha, -1 para izquierda
const int CABEZA_ANGULO_MIN = 60;
const int CABEZA_ANGULO_MAX = 120;
const int MOVIMIENTO_CONTINUO_DELAY = 30;

// --- Variable para controlar el movimiento automático ---
boolean movimientoAutomatico = true;

void setup() {
  // Iniciar comunicación Serial para depuración
  Serial.begin(9600);
  Serial.println("Monitor serial iniciado");

  // Iniciar comunicación Bluetooth HC-06
  myBT.begin(9600); // Velocidad por defecto del HC-06
  Serial.println("Bluetooth HC-06 iniciado. Conecte su dispositivo.");

  // --- Adjuntar servos a sus pines ---
  servoRuedaDerecha.attach(PIN_RUEDA_DERECHA);
  servoRuedaIzquierda.attach(PIN_RUEDA_IZQUIERDA);
  servoBrazoDerecho.attach(PIN_BRAZO_DERECHO);
  servoBrazoIzquierdo.attach(PIN_BRAZO_IZQUIERDO);
  servoCabeza.attach(PIN_CABEZA);

  // --- Posiciones iniciales ---
  StopMovement(); 
  servoBrazoDerecho.write(anguloBrazos);
  servoBrazoIzquierdo.write(anguloBrazos);
  servoCabeza.write(anguloCabeza);

  Serial.println("Robot listo.");
  Serial.println("Comandos disponibles:");
  Serial.println("U: Adelante | D: Atrás | S: Detener");
  Serial.println("R/r: Rueda Derecha Adelante/Detener | L/l: Rueda Izquierda Adelante/Detener");
  Serial.println("W/w: Brazo Derecho Subir/Bajar | A/a: Brazo Izquierdo Subir/Bajar");
  Serial.println("C/c: Cabeza derecha/izquierda | N: Centrar cabeza");
  Serial.println("M: Activar/Desactivar movimiento automático");
  
  // No movemos inicialmente
  StopMovement();
}

void loop() {
  handleBluetoothCommands(); // Procesar comandos Bluetooth
  
  // Solo realizar movimientos automáticos si está activado
  if (movimientoAutomatico) {
    moverCabezaContinuo();     // Movimiento automático de cabeza
    moverBrazosContinuo();     // Movimiento automático de brazos
  }

  delay(MOVIMIENTO_CONTINUO_DELAY);
}

void handleBluetoothCommands() {
  if (myBT.available() > 0) {
    char receivedData = myBT.read();

    // Descartar carácteres no imprimibles como \n o \r
    if (receivedData == '\n' || receivedData == '\r') {
      return;
    }

    // Mostrar comando recibido por Bluetooth
    Serial.print("Comando recibido: ");
    Serial.println(receivedData);
    
    // Confirmación de comando recibido vía Bluetooth
    myBT.print("OK: ");
    myBT.println(receivedData);

    switch (receivedData) {
      case 'U': // Adelante
        NinjaRollForward();
        break;
      case 'D': // Atrás
        NinjaRollBackward();
        break;
      case 'S': // Detener todas las ruedas
        StopMovement();
        break;

      // --- Rueda derecha ---
      case 'R': // Adelante
        servoRuedaDerecha.write(60);
        break;
      case 'r': // Detener
        servoRuedaDerecha.write(90);
        break;

      // --- Rueda izquierda ---
      case 'L': // Adelante
        servoRuedaIzquierda.write(120);
        break;
      case 'l': // Detener
        servoRuedaIzquierda.write(90);
        break;

      // --- Brazo derecho ---
      case 'W': // Subir
        anguloBrazos = min(BRAZO_ANGULO_MAX, anguloBrazos + 10);
        servoBrazoDerecho.write(anguloBrazos);
        break;
      case 'w': // Bajar
        anguloBrazos = max(BRAZO_ANGULO_MIN, anguloBrazos - 10);
        servoBrazoDerecho.write(anguloBrazos);
        break;

      // --- Brazo izquierdo ---
      case 'A': // Subir
        anguloBrazos = min(BRAZO_ANGULO_MAX, anguloBrazos + 10);
        servoBrazoIzquierdo.write(anguloBrazos);
        break;
      case 'a': // Bajar
        anguloBrazos = max(BRAZO_ANGULO_MIN, anguloBrazos - 10);
        servoBrazoIzquierdo.write(anguloBrazos);
        break;

      // --- Cabeza ---
      case 'C': // Girar cabeza a la derecha
        anguloCabeza = min(CABEZA_ANGULO_MAX, anguloCabeza + 10);
        servoCabeza.write(anguloCabeza);
        break;
      case 'c': // Girar cabeza a la izquierda
        anguloCabeza = max(CABEZA_ANGULO_MIN, anguloCabeza - 10);
        servoCabeza.write(anguloCabeza);
        break;
      case 'N': // Centrar cabeza
        anguloCabeza = 90;
        servoCabeza.write(anguloCabeza);
        break;
        
      // --- Control de movimiento automático ---
      case 'M': // Alternar movimiento automático
        movimientoAutomatico = !movimientoAutomatico;
        Serial.print("Movimiento automático: ");
        Serial.println(movimientoAutomatico ? "Activado" : "Desactivado");
        myBT.print("Movimiento automático: ");
        myBT.println(movimientoAutomatico ? "Activado" : "Desactivado");
        break;  

      default:
        Serial.println("Comando no reconocido");
        myBT.println("Comando no reconocido");
        break;
    }
  }
}

// --- Funciones de movimiento general ---
void NinjaRollForward() {
  Serial.println("Moviendo hacia adelante...");
  myBT.println("Adelante");
  servoRuedaDerecha.write(60);    // Rueda derecha hacia adelante
  servoRuedaIzquierda.write(120); // Rueda izquierda hacia adelante
}

void NinjaRollBackward() {
  Serial.println("Moviendo hacia atrás...");
  myBT.println("Atrás");
  servoRuedaDerecha.write(120);   // Rueda derecha hacia atrás
  servoRuedaIzquierda.write(60);  // Rueda izquierda hacia atrás
}

void StopMovement() {
  Serial.println("Deteniendo movimiento de ruedas...");
  myBT.println("Detenido");
  servoRuedaDerecha.write(90);    // Detener rueda derecha
  servoRuedaIzquierda.write(90);  // Detener rueda izquierda
}

// --- Movimiento continuo de cabeza ---
void moverCabezaContinuo() {
  anguloCabeza += direccionCabeza;

  if (anguloCabeza <= CABEZA_ANGULO_MIN) {
    anguloCabeza = CABEZA_ANGULO_MIN;
    direccionCabeza = 1;
  } else if (anguloCabeza >= CABEZA_ANGULO_MAX) {
    anguloCabeza = CABEZA_ANGULO_MAX;
    direccionCabeza = -1;
  }

  servoCabeza.write(anguloCabeza);
}

// --- Movimiento continuo de brazos ---
void moverBrazosContinuo() {
  anguloBrazos += direccionBrazos * VELOCIDAD_BRAZOS_INCREMENTO;

  if (anguloBrazos >= BRAZO_ANGULO_MAX) {
    anguloBrazos = BRAZO_ANGULO_MAX;
    direccionBrazos = -1;
  } else if (anguloBrazos <= BRAZO_ANGULO_MIN) {
    anguloBrazos = BRAZO_ANGULO_MIN;
    direccionBrazos = 1;
  }

  servoBrazoDerecho.write(anguloBrazos);
  servoBrazoIzquierdo.write(anguloBrazos); // Movimiento simétrico
}

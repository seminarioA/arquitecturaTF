#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>

// Teclado 4x4
const byte filas = 4;
const byte columnas = 4;
byte pinesFilas[] = {13, 12, 11, 10};
byte pinesColumnas[] = {9, 8, 7, 6};
char teclas[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
Keypad tecladoUno = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, filas, columnas);

// Pantallas LCD
LiquidCrystal_I2C lcd1(0x20, 16, 2);
LiquidCrystal_I2C lcd2(0x22, 16, 2);

// Pines del LED, sensor PIR, buzzer y sensor de temperatura TMP36
#define LED_PIN 5
#define PIR_PIN 4
#define BUZZER_PIN 3
#define TEMP_PIN A0

// Almacenamiento de la clave ingresada
char clave[5] = "_____";
int indiceClave = 0;

void inicializarPines() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

//PANTALLA LCD
void mostrarLCD1(int col, int row, const char* mensaje, bool limpiarPantalla) {
  lcd1.backlight();
  lcd1.setCursor(col, row);
  if (limpiarPantalla) {
    lcd1.clear();
  }
  lcd1.print(mensaje);
}

void mostrarLCD2(int col, int row, const char* mensaje, bool limpiarPantalla) {
  lcd2.backlight();
  lcd2.setCursor(col, row);
  if (limpiarPantalla) {
    lcd2.clear();
  }
  lcd2.print(mensaje);
}

void pantallaInicioLCDs() {
  mostrarLCD1(0, 0, "Bienvenid@", true);
  mostrarLCD2(0, 0, "Temp: ", true);
  delay(2000);
  mostrarLCD1(0, 0, "Ingrese clave:", true);
  mostrarLCD1(0, 1, " _ _ _ _", false);
}


int pin = 4444;

void presionarTecla(){
  if (char(tecladoUno.getKey()) != NO_KEY) {
    return true; 
}

//TECLAS

//¿EXISTE UN PIN??????
  //NO, CREE UN PIN
  //SI, INGRESE UN PIN
void obtenerTeclaPresionada() {
  char tecla_presionada = tecladoUno.getKey();
  //SI UNA TECLA FUE PRESIONADA Y EL TAMAÑO ES MENOR A 4
  if (tecla_presionada != NO_KEY && indiceClave < 4) {
      clave[indiceClave] = tecla_presionada;
      indiceClave++;   //CONTADOR
    }
    mostrarLCD1(0, 1, clave, false);
  
    if (indiceClave == 4) {
      // Verificar código
      if (verificarCodigoEEPROM(0, clave) || verificarCodigoEEPROM(5, clave)) {
        mostrarLCD1(0, 0, "Codigo valido  ", true);
        digitalWrite(LED_PIN, HIGH); // Encender LED
        delay(2000);
      } else {
        mostrarLCD1(0, 0, "Codigo invalido", true);
        delay(2000);
        mostrarLCD1(0, 0, "Ingrese clave: ", true);
        mostrarLCD1(0, 1, " _ _ _ _", false);
      }
      // Reiniciar clave para nuevo intento
      indiceClave = 0;
      memset(clave, '_', 4);
    }
  }
}
//----------------------------------------------------

//TEMPERATURA
float leerTemp() {
  int lectura = analogRead(TEMP_PIN);
  float voltaje = lectura * 5.0 / 1024.0;
  float temperaturaC = (voltaje - 0.5) * 100.0;
  return temperaturaC;
}

void mostrarTemp() {
  //FUNCION(ARRAY[OBJETO], COLUMNA, FILA, MENSAJE, LIMPIAR PANTALLA);
  mostrarLCD2(0, 1, dtostrf(leerTemp()));
  mostrarLCD2(0, 1, " C");
  delay(100);
//----------------------------------------------------

//SENSOR PIR
int estadoSensorPir() {
  return digitalRead(PIR_PIN);
}

void mostrarEstadoPirLCD() {
  if (estadoSensorPir()) {
    mostrarLCD1(0, 0, "Movimiento", true);
    mostrarLCD1(0, 1, "detectado!", false);
  } else {
    mostrarLCD1(0, 0, "", true); // Limpiar pantalla si no hay movimiento
  }
}

void estadoPirBUZZER() {
  if (estadoSensorPir()) {
    digitalWrite(BUZZER_PIN, HIGH); // Activar buzzer si hay movimiento
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Apagar buzzer si no hay movimiento
  }
}
//----------------------------------------------------

//EEPROM
bool verificarCodigoEEPROM(int direccion, const char* codigo) {
  for (int i = 0; i < 4; i++) {
    if (EEPROM.read(direccion + i) != codigo[i]) {
      return false;
    }
  }
  return true;
}

void guardarCodigo(int direccion, const char* codigo) {
  for (int i = 0; i < 4; i++) {
    EEPROM.write(direccion + i, codigo[i]);
  }
}
//----------------------------------------------------

void setup() {
  Serial.begin(9600);
  lcd1.init();
  lcd2.init();
  guardarCodigo(0, "5678"); // Ejemplo de guardar un código en la dirección 0 de EEPROM
  guardarCodigo(5, "1234"); // Ejemplo de guardar otro código en la dirección 5 de EEPROM
  inicializarPines();
}

void loop() {
  verificarTeclaPresionada();
  mostrarTemp();
  mostrarEstadoPirLCD();
  estadoPirBUZZER();
}

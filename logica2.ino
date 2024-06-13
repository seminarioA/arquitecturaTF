//INCLUIR BIBLIOTECAS
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

//TECLADO 4X4
const byte filas = 4; 
const byte columnas = 4;
byte pinesFilas[]  = {13, 12, 11, 10};
byte pinesColumnas[] = {9, 8, 7, 6};
char teclas[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
Keypad tecladoUno = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, filas, columnas);

//PANTALLAS LCDs
LiquidCrystal_I2C lcd1(0x20, 16, 2);
LiquidCrystal_I2C lcd2(0x22, 16, 2);
LiquidCrystal_I2C* lcds[] = {&lcd1, &lcd2};

// Pines del LED, sensor PIR, buzzer y sensor de temperatura TMP36
#define LED_PIN 5
#define PIR_PIN 4
#define BUZZER_PIN 3
#define TEMP_PIN A0

// Almacena la clave ingresada
char clave[5] = "____";
int indiceClave = 0;

void inicializarPines() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  lcd1.init();
  lcd2.init();
  guardarCodigo(5, "5678");
  inicializarLCDs();
  inicializarPines();
}

void loop() {
  verificarTeclaPresionada();
  mostrarTemperatura();
  verificarSensorPIR();
  pantallaInicioLCDs();
}

//MOSTRAR CONTENIDO EN LCD
void mostrarLCD(LiquidCrystal_I2C &lcd, int col, int row, bool = true) {
  lcd.backlight();
  lcd.setCursor(col, row);
  if (bool) {
    lcd.clear();
  }
  lcd.print(mensaje);
}

//CONTENIDO DE INICIO LCDs
void pantallaInicioLCDs() {
  //FUNCION(ARRAY[OBJETO], COLUMNA, FILA, MENSAJE);
  mostrarLCD(*lcds[0], 0, 0, "Bienvenid@");
  mostrarLCD(*lcds[1], 0, 0, "Temp: ");
  
  millis(2000);
  mostrarLCD(*lcds[0], 0, 0, "Ingrese clave:");
  mostrarLCD(*lcds[0], 0, 1, " _ _ _ _", false);
}


void verificarTeclaPresionada() {
  char tecla_presionada = tecladoUno.getKey();
  
  if (tecla_presionada != NO_KEY) {
    Serial.print("Tecla: ");
    Serial.println(tecla_presionada);
    
    if (indiceClave < 4) {
      clave[indiceClave] = tecla_presionada;
      indiceClave++;
    }

    lcd1.setCursor(0, 1);
    for (int i = 0; i < 4; i++) {
      lcd1.print(clave[i]);
      lcd1.print(' ');
    }
    if (indiceClave == 4) {
      // Verificar código
      if (verificarCodigoEEPROM(0, clave) || verificarCodigoEEPROM(5, clave)) {
        lcd1.clear();
        Serial.println("Código válido");
        lcd1.setCursor(0, 0);
        lcd1.print("Codigo valido  ");  
        
        // Mensaje de código válido
        // Aquí mecanismo de apertura de la puerta
        digitalWrite(LED_PIN, HIGH);
        delay(2000);
      } else {
        Serial.println("Código inválido");
        lcd1.setCursor(0, 0);
        lcd1.print("Codigo invalido");
        //Poner delay
        lcd1.clear();
        lcd1.print("Ingrese clave:");
        lcd1.setCursor(0, 1);
        lcd1.print(" _ _ _ _");
      }
      // Resetear clave para nuevo intento
      indiceClave = 0;
      memset(clave, '_', 4);
      
    }
  }
}


//-----------------------------------------------------
//TEMPERATURA
float leerTemp (){
int lectura = analogRead(TEMP_PIN);
  float voltaje = lectura * 5.0 / 1024.0;
  float temperaturaC = (voltaje - 0.5) * 100.0;  
  return temperaturaC;
}  

void mostrarTemp(){
  //FUNCION(ARRAY[OBJETO], COLUMNA, FILA, MENSAJE, LIMPIAR PANTALLA);
  mostrarLCD(*lcds[1], 0, 1, leerTemp());
  mostrarLCD(*lcds[1], 0, 1, " C");
  delay(100);
}

//-----------------------------------------------------
//SENSOR PIR
int estadoSensorPir(){
  int pirState = digitalRead(PIR_PIN);
  return pirState   
}

void mostrareEstadoPirLCD(){
  if(estadoSensorPir()){
    //FUNCION(ARRAY[OBJETO], COLUMNA, FILA, MENSAJE, LIMPIAR PANTALLA);
    mostrarLCD(*lcds[0], 0, 0, "Movimiento");
    mostrarLCD(*lcds[0], 0, 1, "detectado!", false);
  }
}

void estadoPirBUZZER(){
  if (estadoSensorPir) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}
//-----------------------------------------------------



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

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// Definición de pines y configuración del teclado 4x4
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

// Configuración de los LCDs
LiquidCrystal_I2C lcd1(0x20, 16, 2); // Direccion I2C del primer LCD
LiquidCrystal_I2C lcd2(0x22, 16, 2); // Direccion I2C del segundo LCD (Asegúrate de que las direcciones I2C son correctas y diferentes)

// Pines del LED, sensor PIR, buzzer y sensor de temperatura TMP36
#define LED_PIN 5
#define PIR_PIN 4
#define BUZZER_PIN 3
#define TEMP_PIN A0

// Almacena la clave ingresada
char clave[5] = "____";
int indiceClave = 0;

void setup() {
  Serial.begin(9600);
  guardarCodigo(5, "5678");
  inicializarLCDs();
  inicializarPines();
  
}

void loop() {
  verificarTeclaPresionada();
  mostrarTemperatura();
  verificarSensorPIR();
}


void inicializarLCDs() {
  lcd1.init();
  lcd1.clear();         
  lcd1.backlight();
  lcd1.setCursor(0, 0); 
  lcd1.print("Bienvenid@");
  
  lcd2.init();
  lcd2.clear();         
  lcd2.backlight();
  lcd2.setCursor(0, 0); 
  lcd2.print("Temp: ");
  
  delay(2000);
  lcd1.clear();
  lcd1.setCursor(0, 0);
  lcd1.print("Ingrese clave:");
  lcd1.setCursor(0, 1);
  lcd1.print(" _ _ _ _");

  
}

void inicializarPines() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
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

void mostrarTemperatura() {
  int lectura = analogRead(TEMP_PIN);
  float voltaje = lectura * 5.0 / 1024.0;
  float temperaturaC = (voltaje - 0.5) * 100.0; // Convertir voltaje a temperatura en grados Celsius

  lcd2.setCursor(0, 1);
  lcd2.print(temperaturaC);
  lcd2.print(" C");

  delay(100); // Pequeño retraso para estabilidad
}

void verificarSensorPIR() {
  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    digitalWrite(LED_PIN, HIGH); // Enciende el LED
    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Movimiento");
    lcd1.setCursor(0, 1);
    lcd1.print("detectado!");
    digitalWrite(BUZZER_PIN, HIGH); // Enciende el buzzer
  } else {
    digitalWrite(LED_PIN, LOW); // Apaga el LED
    digitalWrite(BUZZER_PIN, LOW); // Apaga el buzzer
  }
}

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

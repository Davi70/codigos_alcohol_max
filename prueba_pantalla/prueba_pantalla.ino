#include <Wire.h> // Biblioteca para comunicación I2C
#include <Adafruit_GFX.h> // Biblioteca gráfica de Adafruit
#include <Adafruit_SSD1306.h> // Biblioteca para pantallas OLED SSD1306

#define OLED_ADDR 0x3C // Dirección I2C de la pantalla OLED
#define SCREEN_WIDTH 128 // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 32 // Alto de la pantalla OLED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int valorLimite = 500; // Valor límite para activar la alarma
float valorAlcohol;

const int pinNivelAlto = 8;
const int pinNivelMedio = 9;
const int pinNivelBajo = 10;
const int pinNivelMuyBajo = 11;

void setup() {
  // Inicializa la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("No se encontró la pantalla OLED"));
    while (true);
  }
  display.clearDisplay();
  display.display();
  
  // Configura el puerto serial
  Serial.begin(9600);
  
  // Configura los pines de los LED y el zumbador como salidas
  pinMode(pinNivelAlto, OUTPUT);
  pinMode(pinNivelMedio, OUTPUT);
  pinMode(pinNivelBajo, OUTPUT);
  pinMode(pinNivelMuyBajo, OUTPUT);
  pinMode(13, OUTPUT); // Zumbador
}

void loop() {
  valorAlcohol = analogRead(A0); // Lee el valor del sensor de gas
  float porcentaje = (valorAlcohol / 1023.0) * 100; // Calcula el porcentaje

  // Muestra el valor del gas y el porcentaje en la pantalla OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Alcohol: ");
  display.print(valorAlcohol);

  display.setCursor(0, 16);
  display.print("Porcentaje: ");
  display.print(porcentaje, 1); // Muestra el porcentaje con 1 decimal
  display.print(" %");
  display.display(); // Actualiza la pantalla
  
  // Imprime los valores en el monitor serial
  Serial.print("Alcohol: ");
  Serial.println(valorAlcohol);
  Serial.print("Porcentaje: ");
  Serial.println(porcentaje);

  // Control de los LEDs y el zumbador en función del valor de gas
  if (valorAlcohol > valorLimite) {
    digitalWrite(pinNivelAlto, HIGH);
    digitalWrite(pinNivelMedio, LOW);
    digitalWrite(pinNivelBajo, LOW);
    digitalWrite(pinNivelMuyBajo, LOW);
    digitalWrite(13, HIGH); // Enciende el zumbador
  } else if (valorAlcohol > 370) {
    digitalWrite(pinNivelAlto, LOW);
    digitalWrite(pinNivelMedio, HIGH);
    digitalWrite(pinNivelBajo, LOW);
    digitalWrite(pinNivelMuyBajo, LOW);
    digitalWrite(13, LOW); // Apaga el zumbador
  } else if (valorAlcohol > 340) {
    digitalWrite(pinNivelAlto, LOW);
    digitalWrite(pinNivelMedio, LOW);
    digitalWrite(pinNivelBajo, HIGH);
    digitalWrite(pinNivelMuyBajo, LOW);
    digitalWrite(13, LOW); // Apaga el zumbador
  } else {
    digitalWrite(pinNivelAlto, LOW);
    digitalWrite(pinNivelMedio, LOW);
    digitalWrite(pinNivelBajo, LOW);
    digitalWrite(pinNivelMuyBajo, HIGH);
    digitalWrite(13, LOW); // Apaga el zumbador
  }

  delay(400); // Espera 400 ms antes de la siguiente lectura
}

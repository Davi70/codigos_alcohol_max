#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"

// Pantalla OLED con dirección TWI
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;

const int pinNivelAlto = 8;
const int pinNivelMedio = 9;
const int pinNivelBajo = 10;
const int pinNivelMuyBajo = 11;

const int botonAlcohol = 7;
const int botonOximetro = 6;

int valor_limite = 500; // Valor límite para activar alarma
float valor_alcohol;

bool modoAlcohol = false;
bool modoOximetro = false;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

// Funciones de dibujado de corazones
#define LOGO_HEIGHT 24
#define LOGO_WIDTH 24

static const unsigned char PROGMEM logo_bmp[] = {
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00001111, 0b11000011, 0b11110000,
  0b00011111, 0b11100111, 0b11111000,
  0b00111111, 0b11111111, 0b11111100,
  0b01111111, 0b11111111, 0b11111110,
  0b01111111, 0b11111111, 0b11111110,
  0b01111111, 0b11111111, 0b11111110,
  0b01111111, 0b11111111, 0b11111110,
  0b01111111, 0b11111111, 0b11111110,
  0b00111111, 0b11111111, 0b11111100,
  0b00011111, 0b11111111, 0b11111000,
  0b00011111, 0b11111111, 0b11111000,
  0b00001111, 0b11111111, 0b11110000,
  0b00000111, 0b11111111, 0b11100000,
  0b00000011, 0b11111111, 0b11000000,
  0b00000001, 0b11111111, 0b10000000,
  0b00000000, 0b11111111, 0b00000000,
  0b00000000, 0b00111100, 0b00000000,
  0b00000000, 0b00011000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000
};

static const unsigned char PROGMEM logo2_bmp[] = {
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b11100111, 0b00000000,
  0b00000001, 0b11111111, 0b10000000,
  0b00000011, 0b11111111, 0b11000000,
  0b00000011, 0b11111111, 0b11000000,
  0b00000011, 0b11111111, 0b11000000,
  0b00000011, 0b11111111, 0b11000000,
  0b00000001, 0b11111111, 0b10000000,
  0b00000000, 0b11111111, 0b00000000,
  0b00000000, 0b01111110, 0b00000000,
  0b00000000, 0b00111100, 0b00000000,
  0b00000000, 0b00011000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000
};

void setup() {
  // Inicializa y borra la pantalla OLED
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  // Muestra una línea de texto
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Inicializa el puerto serial
  Serial.begin(9600);
  
  // Configura los pines de salida
  pinMode(pinNivelAlto, OUTPUT);
  pinMode(pinNivelMedio, OUTPUT);
  pinMode(pinNivelBajo, OUTPUT);
  pinMode(pinNivelMuyBajo, OUTPUT);
  pinMode(13, OUTPUT); // Pin 13 para el zumbador

  // Configura los pines de los botones como entradas
  pinMode(botonAlcohol, INPUT_PULLUP);
  pinMode(botonOximetro, INPUT_PULLUP);

  // Configura el sensor MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 no encontrado. Verifica el cableado.");
    while (1);
  }
  
  particleSensor.setup(); // Configura el sensor con ajustes predeterminados
  particleSensor.setPulseAmplitudeRed(0x0A); // LED rojo bajo
  particleSensor.setPulseAmplitudeGreen(0); // Apaga LED verde
}

void loop() {
  if (digitalRead(botonAlcohol) == LOW) {
    modoAlcohol = true;
    modoOximetro = false;
  } else if (digitalRead(botonOximetro) == LOW) {
    modoOximetro = true;
    modoAlcohol = false;
  }

  if (modoAlcohol) {
    ejecutarAlcoholimetro();
  } else if (modoOximetro) {
    ejecutarOximetro();
  } else {
    mostrarMenu();
  }
}

void mostrarMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Selecciona una opcion:");
  display.println("1. Alcoholimetro");
  display.println("2. Oximetro");
  display.display();
}
void ejecutarAlcoholimetro() {
  valor_alcohol = analogRead(A0);
  float porcentaje = (valor_alcohol / 1023.0) * 100;

  display.fillRect(0, 0, 128, 16, BLACK);
  display.setCursor(0, 0);
  display.print("Alcohol: ");
  display.print(valor_alcohol);

  display.fillRect(0, 16, 128, 16, BLACK);
  display.setCursor(0, 16);
  display.print("Porcentaje: ");
  display.print(porcentaje, 2);
  display.print("%");
  display.display();

  Serial.print("Alcohol: ");
  Serial.println(valor_alcohol);
  Serial.print("Porcentaje: ");
  Serial.print(porcentaje, 2);
  Serial.println("%");

  if (valor_alcohol > valor_limite) {
    digitalWrite(pinNivel
    }
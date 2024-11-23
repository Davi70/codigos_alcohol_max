#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels // ancho de la pantalla en pixeles.
#define SCREEN_HEIGHT 32 // OLED display height, in pixels // alto de la pantalla en pixeles.

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...

#define OLED_RESET     -1 // si la pantalla no tiene pin de RESET se le coloca -1
#define SCREEN_ADDRESS 0x3C /// direccion i2c del modulo pantalla, usualmente 3C, en ocaciones 3D 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// definicion del mapa de bits de un corazon grande, con 24 x 24 pixeles de tamaño. 
#define LOGO_HEIGHT   24
#define LOGO_WIDTH    24
static const unsigned char PROGMEM logo_bmp[] =           
{ 0b00000000, 0b00000000, 0b00000000,
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
  0b01111111, 0b11111111, 0b11111110, //12
  0b00111111, 0b11111111, 0b11111100, //13
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
  0b00000000, 0b00000000, 0b00000000 };

// definicion del mapa de bits de un corazon pequeño, con 24 x 24 pixeles de tamaño. 
#define LOGO2_HEIGHT   24
#define LOGO2_WIDTH    24
  static const unsigned char PROGMEM logo2_bmp[] =
{ 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b11100111, 0b00000000,
  0b00000001, 0b11111111, 0b10000000,
  0b00000011, 0b11111111, 0b11000000,
  0b00000011, 0b11111111, 0b11000000,
  0b00000011, 0b11111111, 0b11000000, //12
  0b00000011, 0b11111111, 0b11000000, //13
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
  0b00000000, 0b00000000, 0b00000000 };
  
int counter = 0;
#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good. Numero de muestras que toma para calcular el latido por minuto promedio. 
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void setup()
{
  Serial.begin(115200);     // iniciar la interface serial a 115200 de velocidad, por lo que si desea ver los datos de ritmo cardiaco en el motinor serie del software de arduino, debe configurar esta misma velocidad de el monitor serie
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setTextColor(SSD1306_WHITE);      // definicion del color de la pantalla en blanco, o activo. 
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
    display.display();
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop()
{
  long irValue = particleSensor.getIR();



  if (checkForBeat(irValue) == true)        // Estructura if(){} que detecta un latido 
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)      // estructura if(){} que promedia los latidos segun el RATE_SIZE para obtener medidas mas estables
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
       display.clearDisplay();          // 4 instrucicones para ilustrar el valor correspodiente a los latidos del corazon promedio de las ultimas 4 muestras
       display.setTextSize(4);             
       display.setCursor(55,6);             
       display.println(beatAvg);
      testdrawbitmap2(0);              // borra el corazon pequeño
      
      testdrawbitmap(1);                // dibuja el corazon grande
      tone(12,2000,20);                 // (buzzer)emite un sonido en el pin 12 de la plca nano, con 2000 hz de frecuencia, y 20 milisegundos de duracion 
    }
    testdrawbitmap(0);                  // borra el corazon grande
    testdrawbitmap2(1);                 // dibuja el corazon pequeño
  }
// impresion o trasnmision de los paramentros de intensidad de luz infraroja, latidos por minuto, promedio de latidos por minuto al monitor serie
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

// estructura if que determina si no hay un dedo colocado en el sensor
  if (irValue < 50000){
    Serial.print(" No finger?");
      display.clearDisplay();
       display.setTextSize(2);             // Normal 2:1 pixel scale
       display.setCursor(0,0);             // Start at top-left corner
       display.print("Ubique su dedo");
       display.display();
  }
  Serial.println();
}

// Metodo que dibuja el corazon grande en la pantalla OLED, el argumento (color) tiene 2 posibles valores (0 o 1) si esta en 0 borra los pixeles, si esta en 1 activa los pixeles.
void testdrawbitmap(int color) {
  display.drawBitmap(
    ((display.width()  - LOGO_WIDTH ) / 5),
    4,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, color);
  display.display();
}

// Metodo que dibuja el corazon pequeño en la pantalla OLED, el argumento (color) tiene 2 posibles valores (0 o 1) si esta en 0 borra los pixeles, si esta en 1 activa los pixeles.
void testdrawbitmap2(int color) {
  display.drawBitmap(
    ((display.width()  - LOGO_WIDTH ) / 5),
    4,
    logo2_bmp, LOGO2_WIDTH, LOGO2_HEIGHT, color);
  display.display();
}




// ################################################################################
// # CO2-Sensor with Display and Traffic Light                                    #
// # based on Wemos D1 mini, MH-Z19B CO2-Sensor, OLED-Shield 64x48, Neopixel LED  #
// #############################################################################ch#

#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// Define OLED Display - Defined a 64x128 display although just a 64x48 display is installed. This enables scrolling of longer text.
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Correct settings in Adafruit_SSD1306.h!");
#endif

// Define Neopixel LED
#define PIXEL_PIN   D8      // RGB LED Pin
#define PIXEL_COUNT 1       // Number of pixels
#define PIXEL_BRIGHTNESS 1  // 0-255
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);  // GRB Neopixel

// Define CO2-Sensor
int CO2value;                      // CO2 in ppm
int readInterval = 10000;          // in Milliseconds
SoftwareSerial co2Serial(D6, D5);  // MH-Z19B RX and TX Pin

// MH-Z19B Commands from Sensor datasheet:
// 0x86 Read CO2 concentration
// byte cmdread[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
// Self calibration:
// After the module works for some time, it can judge the zero point intelligently and do the zero calibration automatically.
// The calibration cycle is every 24 hours since the module is power on. The zero point is 400ppm.
// Default is ON.
// 0x79 ON/OFF Self calibration function for zero point (Byte3 0xA0=ON,0x00=OFF) 
// byte cmdcalib[9] = {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86};
// byte cmdcalib[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};

int readCO2sensor() {  // Communication with MH-Z19B CO2-Sensor
  byte cmdread[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  byte answer[9];
  co2Serial.write(cmdread, 9);
  co2Serial.readBytes(answer, 9);
  if (answer[0] != 0xFF) return -1;
  if (answer[1] != 0x86) return -1;
  int answerHigh = (int) answer[2];  // CO2 High Byte
  int answerLow = (int) answer[3];   // CO2 Low Byte
  int correction = -10;  // correction in case the sensor self calibration doesn't show 400 at startup.
  int ppm = (answerHigh * 256) + answerLow + correction;
  return ppm;  // Return CO2 value in ppm
}


void setup() {
  // Onboard LED - Off
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialize display
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize with the I2C addr 0x3C (for the 64x48)
  display.clearDisplay();

  // Initialize Neopixel LED
  strip.begin();
  strip.show();
  strip.setBrightness(PIXEL_BRIGHTNESS);

  // Initialize CO2-Sensor
  co2Serial.begin(9600);
}


void loop() {
  CO2value = readCO2sensor();   // read data from MH-Z19B CO2-Sensor
  //Serial.println(CO2value);

  // LED traffic light colors adjusted to Neopixel model
  uint32_t green = strip.Color(0, 255, 0);
  uint32_t yellow = strip.Color(255, 180, 0);
  uint32_t orange = strip.Color(255, 85, 0);
  uint32_t red = strip.Color(255, 0, 0);
  uint32_t purple = strip.Color(125, 0, 255);

  // Green <700, Yellow >=700, Orange >=850, Red >=1000, Purple >=1400
  if (CO2value >= 1400)
    strip.setPixelColor(0, purple);
  else if (CO2value >= 1000 && CO2value < 1400)
    strip.setPixelColor(0, red);
  else if (CO2value >= 850 && CO2value < 1000)
    strip.setPixelColor(0, orange);
  else if (CO2value >= 700 && CO2value < 850)
    strip.setPixelColor(0, yellow);
  else
    strip.setPixelColor(0, green);
  strip.show();

  display.clearDisplay();
  display.setTextColor(WHITE);

  // Display current CO2 value and warning >1000 ppm
  if (CO2value >= 1000){
    display.setCursor(40,17);  // x, y need some tuning as the virtual display is larger than the physical display
    display.setTextSize(1);
    display.print(CO2value);
    display.println(" ppm");

    display.setCursor(33,32);  // x, y
    display.setTextSize(2);
    display.println("Bitte");  // "Please"
    display.setCursor(30,50);  // x, y
    display.setTextSize(2);
    display.print("l\201ften!");  // "ventilate"
    display.display();
    delay(100);
    display.startscrollleft(0x06, 0x07);
    delay(850);
    display.stopscroll();
    delay(250);
    display.startscrollright(0x06, 0x07);
    delay(850);
    display.stopscroll();
    }
  // Display current CO2 value
  else {
    display.setTextSize(2);
    display.setCursor(45,33);  // x, y
    if (CO2value < 0) {
      display.println("---");
      }
    else {
      display.println(CO2value);
      }
    display.setTextSize(1);
    display.setCursor(77,50);  // x, y
    display.println("ppm");
    display.display();
    }
  delay(readInterval);
}

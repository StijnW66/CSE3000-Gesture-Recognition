#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int vals[128];

void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  Serial.begin(9600);
  
  // Init display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);

  for (int i = 0; i < 128; i++) {
        vals[i] = analogRead(A1) / 15;
        delay(5);
  }
}

void loop() {
    display.clearDisplay();

    int val = analogRead(A1) / 15;

    for (int i = 0; i < 127; i++) {
        vals[i] = vals[i+1];
    }
    vals[127] = val;

    for (int i = 0; i < 128; i++) {
        display.drawLine(i, 0, i, vals[i], WHITE);
    }
    
    display.display();
}
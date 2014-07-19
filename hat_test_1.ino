#include <Adafruit_NeoPixel.h>

int incomingByte = 0;

#define BUFFERLEN 35
uint8_t buffer[BUFFERLEN];
uint32_t testColor = 0b00000000000011110000000000000000; //Red

uint8_t colTracker = 0;
uint8_t rawLen = 48;
uint8_t rawString[] = 
    {
    0b01111111,
    0b00001000,
    0b00001000,
    0b01111111,
    0b00000000,
    0b01111110,
    0b00010001,
    0b00010001,
    0b01111110,
    0b00000000,
    0b00111110,
    0b01000001,
    0b01000001,
    0b00100010,
    0b00000000,
    0b01111111,
    0b00010100,
    0b00100010,
    0b01000001,
    0b00000000,
    0b01111110,
    0b00010001,
    0b00010001,
    0b01111110,
    0b00000000,
    0b01111111,
    0b01000001,
    0b01000001,
    0b00111110,
    0b00000000,
    0b01111110,
    0b00010001,
    0b00010001,
    0b01111110,
    0b00000000,
    0b00000111,
    0b00001000,
    0b01110000,
    0b00001000,
    0b00000111,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
    };


// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip0 = Adafruit_NeoPixel(35, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(35, 19, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(35, 18, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(35, 17, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(35, 16, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip5 = Adafruit_NeoPixel(35, 15, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip6 = Adafruit_NeoPixel(35, 14, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  
  strip0.begin();
  strip0.show(); // Initialize all pixels to 'off'
  strip1.begin();
  strip1.show(); // Initialize all pixels to 'off'
  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off'
  strip3.begin();
  strip3.show(); // Initialize all pixels to 'off'
  strip4.begin();
  strip4.show(); // Initialize all pixels to 'off'
  strip5.begin();
  strip5.show(); // Initialize all pixels to 'off'
  strip6.begin();
  strip6.show(); // Initialize all pixels to 'off'  
  
  //fill buffer with nothing
  for (uint8_t i = 0; i<BUFFERLEN; i++) {
      buffer[i] = 0b00000000;
  }
}

void loop() {
  if (Serial.available() > 0) {
        // read the incoming byte:
        incomingByte = Serial.read();

        // say what you got:
        Serial.print("I received: ");
        Serial.println(incomingByte, DEC);
  }
  //Serial.println("Hello world!");
  // Some example procedures showing how to display to the pixels:
  
  //Latch from the last loop:
  strip0.show();
  strip1.show();
  strip2.show();
  strip3.show();
  strip4.show();
  strip5.show();
  strip6.show();

  //Shift the framebuffer
  for (uint8_t i=BUFFERLEN-1; i>0; i--) {
      buffer[i] = buffer[i-1];
  }
  //Fill the initial column
  buffer[0] = rawString[colTracker];
  //Increment the tracking
  if (++colTracker >= rawLen) { colTracker = 0; }

  //push data to pixels (will latch next loop)
  for (uint8_t i=0; i<BUFFERLEN; i++) {
      if (1<<0 & buffer[i]) { strip0.setPixelColor(i,testColor); }
      else { strip0.setPixelColor(i,0); }
      if (1<<1 & buffer[i]) { strip1.setPixelColor(i,testColor); }
      else { strip1.setPixelColor(i,0); }
      if (1<<2 & buffer[i]) { strip2.setPixelColor(i,testColor); }
      else { strip2.setPixelColor(i,0); }
      if (1<<3 & buffer[i]) { strip3.setPixelColor(i,testColor); }
      else { strip3.setPixelColor(i,0); }
      if (1<<4 & buffer[i]) { strip4.setPixelColor(i,testColor); }
      else { strip4.setPixelColor(i,0); }
      if (1<<5 & buffer[i]) { strip5.setPixelColor(i,testColor); }
      else { strip5.setPixelColor(i,0); }
      if (1<<6 & buffer[i]) { strip6.setPixelColor(i,testColor); }
      else { strip6.setPixelColor(i,0); }
  }
  delay(80);


}

// Cascade a pixel down the row
void cascade(Adafruit_NeoPixel strip, uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    if (i > 0) { strip.setPixelColor(i-1, strip.Color(0, 0, 0)); }
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}



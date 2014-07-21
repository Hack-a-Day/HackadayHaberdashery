#include <Adafruit_NeoPixel.h>
#include <avr/pgmspace.h>


int incomingByte = 0;

//Keep track of millis for loop execution
uint32_t alarm;
#define DELAY 80

//Message state declarations and variable
#define INCHAR 0
#define INSPACE 1
#define COLCLEAR 2
#define NEXTCHAR 3
uint8_t msgState = NEXTCHAR;

//Message variables
boolean serialMsgReady = false;  //A message was received over serial
boolean serialMsgScrolling = false; //A message received over serial is currently being displayed
uint8_t serialMsgIdx = 0;
uint8_t msgLen = 11; // How many letters in the message (may not need this if zero terminated)
#define MSGCUSTOMARRAYLEN 20
uint8_t msgCustom[MSGCUSTOMARRAYLEN] = { 66, 97, 99, 111, 110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //Stores the custom message (zero terminated)
uint8_t msgIdx = 0; //Which letter are we on?
uint8_t chrIdx = 0; //WHich column of this letter's font are we on?
uint8_t nextCol = 0; // Next column pixels (Doesn't need to be global but whatevs)
uint8_t clearIdx = 0;

#define BUFFERLEN 35
uint8_t buffer[BUFFERLEN];
uint32_t testColor = 0b00000000001111110000000000000000; //Red

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

//Font file doesn't use RAM
static const char PROGMEM  font5x8[] = {
  0x00, 0x00, 0x00, 0x00, 0x00,// (space)
  0x00, 0x00, 0x5F, 0x00, 0x00,// !
  0x00, 0x07, 0x00, 0x07, 0x00,// "
  0x14, 0x7F, 0x14, 0x7F, 0x14,// #
  0x24, 0x2A, 0x7F, 0x2A, 0x12,// $
  0x23, 0x13, 0x08, 0x64, 0x62,// %
  0x36, 0x49, 0x55, 0x22, 0x50,// &
  0x00, 0x05, 0x03, 0x00, 0x00,// '
  0x00, 0x1C, 0x22, 0x41, 0x00,// (
  0x00, 0x41, 0x22, 0x1C, 0x00,// )
  0x08, 0x2A, 0x1C, 0x2A, 0x08,// *
  0x08, 0x08, 0x3E, 0x08, 0x08,// +
  0x00, 0x50, 0x30, 0x00, 0x00,// ,
  0x08, 0x08, 0x08, 0x08, 0x08,// -
  0x00, 0x30, 0x30, 0x00, 0x00,// .
  0x20, 0x10, 0x08, 0x04, 0x02,// /
  0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
  0x00, 0x42, 0x7F, 0x40, 0x00,// 1
  0x42, 0x61, 0x51, 0x49, 0x46,// 2
  0x21, 0x41, 0x45, 0x4B, 0x31,// 3
  0x18, 0x14, 0x12, 0x7F, 0x10,// 4
  0x27, 0x45, 0x45, 0x45, 0x39,// 5
  0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
  0x01, 0x71, 0x09, 0x05, 0x03,// 7
  0x36, 0x49, 0x49, 0x49, 0x36,// 8
  0x06, 0x49, 0x49, 0x29, 0x1E,// 9
  0x00, 0x36, 0x36, 0x00, 0x00,// :
  0x00, 0x56, 0x36, 0x00, 0x00,// ;
  0x00, 0x08, 0x14, 0x22, 0x41,// <
  0x14, 0x14, 0x14, 0x14, 0x14,// =
  0x41, 0x22, 0x14, 0x08, 0x00,// >
  0x02, 0x01, 0x51, 0x09, 0x06,// ?
  0x32, 0x49, 0x79, 0x41, 0x3E,// @
  0x7E, 0x11, 0x11, 0x11, 0x7E,// A
  0x7F, 0x49, 0x49, 0x49, 0x36,// B
  0x3E, 0x41, 0x41, 0x41, 0x22,// C
  0x7F, 0x41, 0x41, 0x22, 0x1C,// D
  0x7F, 0x49, 0x49, 0x49, 0x41,// E
  0x7F, 0x09, 0x09, 0x01, 0x01,// F
  0x3E, 0x41, 0x41, 0x51, 0x32,// G
  0x7F, 0x08, 0x08, 0x08, 0x7F,// H
  0x00, 0x41, 0x7F, 0x41, 0x00,// I
  0x20, 0x40, 0x41, 0x3F, 0x01,// J
  0x7F, 0x08, 0x14, 0x22, 0x41,// K
  0x7F, 0x40, 0x40, 0x40, 0x40,// L
  0x7F, 0x02, 0x04, 0x02, 0x7F,// M
  0x7F, 0x04, 0x08, 0x10, 0x7F,// N
  0x3E, 0x41, 0x41, 0x41, 0x3E,// O
  0x7F, 0x09, 0x09, 0x09, 0x06,// P
  0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
  0x7F, 0x09, 0x19, 0x29, 0x46,// R
  0x46, 0x49, 0x49, 0x49, 0x31,// S
  0x01, 0x01, 0x7F, 0x01, 0x01,// T
  0x3F, 0x40, 0x40, 0x40, 0x3F,// U
  0x1F, 0x20, 0x40, 0x20, 0x1F,// V
  0x7F, 0x20, 0x18, 0x20, 0x7F,// W
  0x63, 0x14, 0x08, 0x14, 0x63,// X
  0x03, 0x04, 0x78, 0x04, 0x03,// Y
  0x61, 0x51, 0x49, 0x45, 0x43,// Z
  0x00, 0x00, 0x7F, 0x41, 0x41,// [
  0x02, 0x04, 0x08, 0x10, 0x20,// "\"
  0x41, 0x41, 0x7F, 0x00, 0x00,// ]
  0x04, 0x02, 0x01, 0x02, 0x04,// ^
  0x40, 0x40, 0x40, 0x40, 0x40,// _
  0x00, 0x01, 0x02, 0x04, 0x00,// `
  0x20, 0x54, 0x54, 0x54, 0x78,// a
  0x7F, 0x48, 0x44, 0x44, 0x38,// b
  0x38, 0x44, 0x44, 0x44, 0x00,// c
  0x38, 0x44, 0x44, 0x48, 0x7F,// d
  0x38, 0x54, 0x54, 0x54, 0x18,// e
  0x08, 0x7E, 0x09, 0x01, 0x02,// f
  0x08, 0x14, 0x54, 0x54, 0x3C,// g
  0x7F, 0x08, 0x04, 0x04, 0x78,// h
  0x00, 0x44, 0x7D, 0x40, 0x00,// i
  0x20, 0x40, 0x44, 0x3D, 0x00,// j
  0x00, 0x7F, 0x10, 0x28, 0x44,// k
  0x00, 0x41, 0x7F, 0x40, 0x00,// l
  0x7C, 0x04, 0x18, 0x04, 0x78,// m
  0x7C, 0x08, 0x04, 0x04, 0x78,// n
  0x38, 0x44, 0x44, 0x44, 0x38,// o
  0x7C, 0x14, 0x14, 0x14, 0x08,// p
  0x08, 0x14, 0x14, 0x18, 0x7C,// q
  0x7C, 0x08, 0x04, 0x04, 0x08,// r
  0x48, 0x54, 0x54, 0x54, 0x20,// s
  0x04, 0x3F, 0x44, 0x40, 0x20,// t
  0x3C, 0x40, 0x40, 0x20, 0x7C,// u
  0x1C, 0x20, 0x40, 0x20, 0x1C,// v
  0x3C, 0x40, 0x30, 0x40, 0x3C,// w
  0x44, 0x28, 0x10, 0x28, 0x44,// x
  0x0C, 0x50, 0x50, 0x50, 0x3C,// y
  0x44, 0x64, 0x54, 0x4C, 0x44,// z
  0x00, 0x08, 0x36, 0x41, 0x00,// {
  0x00, 0x00, 0x7F, 0x00, 0x00,// |
  0x00, 0x41, 0x36, 0x08, 0x00,// }
  0x08, 0x08, 0x2A, 0x1C, 0x08,// ->
  0x08, 0x1C, 0x2A, 0x08, 0x08 // <-
};

//RAM to hold one letter of font
uint8_t chrBuf[5] = { 0, 0, 0, 0, 0 };

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstreamweak (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip0 = Adafruit_NeoPixel(35, 14, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(35, 15, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(35, 16, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(35, 17, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(35, 18, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip5 = Adafruit_NeoPixel(35, 19, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip6 = Adafruit_NeoPixel(35, 6, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  
  //initialize strips
  strip0.begin();
  strip1.begin();
  strip2.begin();
  strip3.begin();
  strip4.begin();
  strip5.begin();
  strip6.begin();
  
  //Latch strips
  latch();
  
  //fill buffer with nothing
  for (uint8_t i = 0; i<BUFFERLEN; i++) {
      buffer[i] = 0b00000000;
  }
  
  //Set the column shift timer for the first time
  alarm = millis() + DELAY;
}

void loop() {
  //Some serial stuff for testing (will be used eventually)

  if (Serial.available() > 0) {
        // read the incoming byte:
        incomingByte = Serial.read();
        
        

        // say what you got:
        Serial.print("I received: ");
        Serial.println(incomingByte, DEC);
        readFont(incomingByte);
        Serial.println(chrBuf[0],HEX);
        Serial.println(chrBuf[1],HEX);
        Serial.println(chrBuf[2],HEX);
        Serial.println(chrBuf[3],HEX);
        Serial.println(chrBuf[4],HEX);
        
        //Only overwrite the message buffer if a serial message is not currently scrolling
        if (!serialMsgScrolling) {
          if (incomingByte == 10) {
            //This is a newline character that terminates the string
            //Have a zero added to the array by resetting value of incomingByte to 0
            msgCustom[serialMsgIdx] = 0;
            //Reset index for next time
            serialMsgIdx = 0;
          }
          else { msgCustom[serialMsgIdx++] = incomingByte; }
          
          //Crude way to prevent index overflow
          if (serialMsgIdx >= MSGCUSTOMARRAYLEN) { serialMsgIdx = 0; }
        }
  }
  
  if (millis() >= alarm) {
  
    //Latch from previous loop
    latch();
    
    //Reset the alarm for next time
    alarm = millis() + DELAY;
    
    //TODO: msgRepeat
    //TODO: Load message from RAM
    
    switch(msgState) {
      case INCHAR:
        //Are we in the middle of a character?
        nextCol = chrBuf[chrIdx++];
        
        //Change state on overflow
        if (chrIdx >= 5) {
          if ((msgIdx >= MSGCUSTOMARRAYLEN) || (msgCustom[msgIdx] == 0)) { msgState = COLCLEAR; }
          else { msgState = INSPACE; }
        }
        break;
        
      case INSPACE:
        //Are we pushing space between two characters?
        nextCol = 0;
        msgState = NEXTCHAR;
        break;
        
      case COLCLEAR:
        //Are we clearing the display by columns?
        nextCol = 0;
        if (clearIdx++ >= BUFFERLEN) {
          //Reset all variables
          msgIdx = 0; //Which letter are we on?
          chrIdx = 0; //WHich column of this letter's font are we on?
          nextCol = 0; // Next column pixels (Doesn't need to be global but whatevs)
          clearIdx = 0;
          msgState = NEXTCHAR;
        }
        break;
        
      default:
        //Otherwise go to the next letter
        
        //Load next character from font
        readFont(msgCustom[msgIdx++]);
        
        //Set pixels to be pushed
        nextCol = chrBuf[0];
        
        //Set character column index for next loop
        chrIdx = 1;
        
        //Set state for next loop
        msgState = INCHAR;
    
    //Push next column of the character
    //Flag space between characters if we overflowed columns
    }
  
    //Shift all columns and add a new one to the beginning
    pushColumn(nextCol);
    
    //Increment the tracking
    if (++colTracker >= rawLen) { colTracker = 0; }

  }
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

void latch(void) {
  //Latch all strips:
  strip0.show();
  strip1.show();
  strip2.show();
  strip3.show();
  strip4.show();
  strip5.show();
  strip6.show();
}

void readFont(uint8_t letter) {
  //Filter for invalid
  if ((letter < 32) || (letter > 127)) { return; }
  //Fill the buffer with this characters font
  //Adjust by 32 to match ASCII numbers
  chrBuf[0] = pgm_read_byte_near(font5x8 + (5 *(letter-32)));
  chrBuf[1] = pgm_read_byte_near(font5x8 + (5 *(letter-32)) + 1);
  chrBuf[2] = pgm_read_byte_near(font5x8 + (5 *(letter-32)) + 2);
  chrBuf[3] = pgm_read_byte_near(font5x8 + (5 *(letter-32)) + 3);
  chrBuf[4] = pgm_read_byte_near(font5x8 + (5 *(letter-32)) + 4);
}

void pushColumn(uint8_t newColumn) {
  
  //Shift the framebuffer
  for (uint8_t i=BUFFERLEN-1; i>0; i--) {
      buffer[i] = buffer[i-1];
  }
  
  //Fill the initial column
  buffer[0] = newColumn;
  
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
}


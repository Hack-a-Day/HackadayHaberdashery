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
#define PACMAN 3
#define NEXTCHAR 4
#define SCANNER 5
uint8_t msgState = NEXTCHAR;
//uint8_t msgState = SCANNER;
#define STANDARDREPEAT 3
int8_t msgRepeat = STANDARDREPEAT;
uint8_t stockMsgTracker = 0;



//Message variables
boolean serialMsgReady = false;  //A message was received over serial
boolean serialMsgScrolling = false; //A message received over serial is currently being displayed
uint8_t serialMsgIdx = 0;
uint8_t msgLen = 11; // How many letters in the message (may not need this if zero terminated)
#define MSGCUSTOMARRAYLEN 40
uint8_t msgBuffer[MSGCUSTOMARRAYLEN] = { 66, 97, 99, 111, 110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //Stores the currently scrolling message (zero terminated)
uint8_t msgCustom[MSGCUSTOMARRAYLEN] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Stores incoming custom messages
uint8_t msgIdx = 0; //Which letter are we on?
uint8_t chrIdx = 0; //WHich column of this letter's font are we on?
uint8_t nextCol = 0; // Next column pixels (Doesn't need to be global but whatevs)
uint8_t clearIdx = 0;

//Stock messages
static const uint8_t PROGMEM message0[] = { "Hackaday" };
static const uint8_t PROGMEM message1[] = { "WiFi Hat" };
static const uint8_t PROGMEM message2[] = { "DEFCON"};
#define MSGCOUNT 3
static const uint8_t* msgPointers[MSGCOUNT] = { message0, message1, message2 };

//PACMAN Animations
uint8_t blinky[] = { 0x7c, 0x3e, 0x7f, 0x3f, 0x7f, 0x3e, 0x7c };
uint8_t pacman0[] = { 0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c };
uint8_t pacman1[] = { 0x00, 0x22, 0x77, 0x7f, 0x7f, 0x3e, 0x1c };
uint8_t pacman2[] = { 0x00, 0x00, 0x41, 0x63, 0x77, 0x3e, 0x1c };

//Larson Scanner variables
int8_t livePixel = 0;
int8_t scanDirection = 1;
#define SCANLIMIT 15
#define SCANLOC 8
#define SCANNERDELAY 40

#define BUFFERLEN 35
uint8_t buffer[BUFFERLEN];
#define CUSTOMCOLOR 0b00000000000000000011111100000000
#define COLORSINPALETTE 7
//We never want more than 0xAA total intensity
uint32_t colorsLow[8] = {
  0xAA0000, // red
  0x00AA00, // green
  0x0000AA, // blue
  0x555500, // yellow
  0x550055, // magenta
  0x005555, // cyan
  0x383838  // white
};
uint8_t stockColor = COLORSINPALETTE;  //index for pulling rotating stock colors
uint32_t curColor = colorsLow[stockColor];

//PACMAN variables
#define PACMANSTDSTART -1
int8_t pacmanStart = PACMANSTDSTART;
#define BLINKYSTART -11
#define BLINKYEYEONE -13
#define BLINKYEYETWO -15
#define PACMANCOLOR 0x664400
#define BLINKYCOLOR 0xAA0000

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
  0x38, 0x44, 0x44, 0x44, 0x44,// c
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
Adafruit_NeoPixel strip0 = Adafruit_NeoPixel(32, 7, NEO_GRB + NEO_KHZ800); //Top
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(32, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(32, 5, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(32, 14, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(32, 15, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip5 = Adafruit_NeoPixel(32, 16, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip6 = Adafruit_NeoPixel(32, 17, NEO_GRB + NEO_KHZ800);  //Bottom

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
  
  //load first message
  loadStockMsg();
  
  //Set the column shift timer for the first time
  alarm = millis() + DELAY;
}

void loop() {
  //Some serial stuff for testing (will be used eventually)

  if (Serial.available() > 0) {
        // read the incoming byte:
        incomingByte = Serial.read();
        
        

        // say what you got:
        /*
        Serial.print("I received: ");
        Serial.println(incomingByte, DEC);
        readFont(incomingByte);
        Serial.println(chrBuf[0],HEX);
        Serial.println(chrBuf[1],HEX);
        Serial.println(chrBuf[2],HEX);
        Serial.println(chrBuf[3],HEX);
        Serial.println(chrBuf[4],HEX);
        */
        
        //Only overwrite the message buffer if a serial message is not currently scrolling
        if (!serialMsgScrolling) {
          if (incomingByte == 10) {
            //This is a newline character that terminates the string
            //Have a zero added to the array by resetting value of incomingByte to 0
            msgCustom[serialMsgIdx] = 0;
            //Reset index for next time
            serialMsgIdx = 0;
            //Flag that a message is ready
            serialMsgReady = true;
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
    
    //State machine for handling message rotation and display behavior    
    switch(msgState) {
      case INCHAR:
        //Are we in the middle of a character?
        nextCol = chrBuf[chrIdx++];
        
        //Change state on overflow
        if (chrIdx >= 5) {
          if ((msgIdx >= MSGCUSTOMARRAYLEN) || (msgBuffer[msgIdx] == 0)) { msgState = COLCLEAR; }
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
          resetMsgVars();
        }
        break;
        
      case PACMAN:
        pacman();
        break;
        
      case SCANNER:
        alarm = millis() + SCANNERDELAY;
        larsonScanner();
        break;
        
      default:   //This is where NEXTCHAR is executed (it doesn't have a "case" entry
        //Otherwise go to the next letter
        
        //Check for custom message
        if (msgIdx == 0) {
          if (serialMsgReady) {
            //Change message color
            curColor = CUSTOMCOLOR;
            //Copy serial message into message buffer
            for (uint8_t i=0; i<MSGCUSTOMARRAYLEN; i++) {
              msgBuffer[i] = msgCustom[i];
            }
            msgRepeat = STANDARDREPEAT;
            serialMsgScrolling = true;
            serialMsgReady = false;
          }
          //repeat tracker for custom messages
          else if (serialMsgScrolling) {
            if (--msgRepeat <= 0) {
              msgRepeat = STANDARDREPEAT;
              loadStockMsg(); //reload a stock message
              serialMsgScrolling = false;
              //Reset Color
              curColor = colorsLow[stockColor];
            }
          }
          //repeat tracker for stock messages
          else {
            if (--msgRepeat <= 0) {
              msgRepeat = STANDARDREPEAT;
              if(++stockMsgTracker >= MSGCOUNT) {
                //Reset tracking variables
                resetMsgVars();
                //Launch animation before rolling over the message tracker
                msgState = PACMAN;
                break;
              }
              else if (stockMsgTracker == 1) {
                livePixel = 0;
                scanDirection = 1;
                msgRepeat = 18;
                msgState = SCANNER;
                break;
              }
              loadStockMsg();
            }
          }
        }
        
        
        //Load next character from font
        readFont(msgBuffer[msgIdx++]);
        
        //Set pixels to be pushed
        nextCol = chrBuf[0];
        
        //Set character column index for next loop
        chrIdx = 1;
        
        //Set state for next loop
        msgState = INCHAR;
        

    }
  
    //Shift all columns and add a new one to the beginning
    if (msgState != SCANNER) { pushColumn(nextCol); }

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

//Shift the framebuffer by one column
void shiftBuffer(void) {
  for (uint8_t i=BUFFERLEN-1; i>0; i--) {
      buffer[i] = buffer[i-1];
  }
}

void pushColumn(uint8_t newColumn) {
  
  //Shift the framebuffer
  shiftBuffer();
  
  //Fill the initial column
  buffer[0] = newColumn;
  
  //push data to pixels (will latch next loop)
  for (uint8_t i=0; i<BUFFERLEN; i++) {
    
    //Colors for Pacman animations
    if (msgState == PACMAN) {
      //Normally we want blinky's color
      curColor = BLINKYCOLOR;
      //If we're about to push a pixel for pacman change the color
      if ((i <= pacmanStart) && (i > pacmanStart - 7)) { curColor = PACMANCOLOR; }   
    }
    
    if (1<<0 & buffer[i]) { strip0.setPixelColor(i,curColor); }
    else { strip0.setPixelColor(i,0); }
    if (1<<1 & buffer[i]) { strip1.setPixelColor(i,curColor); }
    else { strip1.setPixelColor(i,0); }
    //Dirty hack to draw Blinky's white eyes
    if ((msgState == PACMAN) && ((i == pacmanStart + BLINKYSTART - 2) || (i == pacmanStart + BLINKYSTART-4))) {strip2.setPixelColor(i,colorsLow[6]);}
    else if (1<<2 & buffer[i]) { strip2.setPixelColor(i,curColor); }
    else { strip2.setPixelColor(i,0); }
    if (1<<3 & buffer[i]) { strip3.setPixelColor(i,curColor); }
    else { strip3.setPixelColor(i,0); }
    if (1<<4 & buffer[i]) { strip4.setPixelColor(i,curColor); }
    else { strip4.setPixelColor(i,0); }
    if (1<<5 & buffer[i]) { strip5.setPixelColor(i,curColor); }
    else { strip5.setPixelColor(i,0); }
    if (1<<6 & buffer[i]) { strip6.setPixelColor(i,curColor); }
    else { strip6.setPixelColor(i,0); }
  } 
}

void larsonScanner(void) {
  //Draw pixel
  strip6.setPixelColor(SCANLOC+livePixel,0xFF0000);
  
  //TODO: fading tail
  if (scanDirection > 0) {
    if (livePixel > 0) { strip6.setPixelColor(SCANLOC+livePixel-1,0x240000); }
    if (livePixel == 1) { 
      strip6.setPixelColor(SCANLOC+livePixel+1,0x070000);
      strip6.setPixelColor(SCANLOC+livePixel+2,0x000000);
    }
    if (livePixel > 1) { strip6.setPixelColor(SCANLOC+livePixel-2,0x280000); } 
    if (livePixel > 2) { strip6.setPixelColor(SCANLOC+livePixel-3,0x070000); } 
    if (livePixel > 3) { strip6.setPixelColor(SCANLOC+livePixel-4,0); } 
  }
  else {
    if (livePixel < SCANLIMIT-1) {strip6.setPixelColor(SCANLOC+livePixel+1, 0xCC0000); }
    if (livePixel == SCANLIMIT-2) { 
      strip6.setPixelColor(SCANLOC+livePixel-1,0x440000);
      strip6.setPixelColor(SCANLOC+livePixel-2,0x000000);
    }
    if (livePixel < SCANLIMIT-2) { strip6.setPixelColor(SCANLOC+livePixel+2,0x880000); } 
    if (livePixel < SCANLIMIT-3) { strip6.setPixelColor(SCANLOC+livePixel+3,0x440000); } 
    if (livePixel < SCANLIMIT-4) { strip6.setPixelColor(SCANLOC+livePixel+4,0); } 
  }
  
  //Move pixel
  livePixel += scanDirection;
  //Check for direction chan
  if (livePixel < 0) {
    scanDirection = 1;
    livePixel = 1;
    --msgRepeat;
  }
  else if (livePixel >= SCANLIMIT) {
    scanDirection = -1;
    livePixel = SCANLIMIT - 2;
    --msgRepeat;
  }
  
  if (msgRepeat <= 0) {
    msgRepeat = STANDARDREPEAT;
    resetMsgVars();
    stockMsgTracker = 1;
    loadStockMsg();
  }  
}

void pacman(void) {
  //Increment the counter -- exit if we've overflowed
  if (++pacmanStart >= MSGCUSTOMARRAYLEN-BLINKYSTART+6) {
    //Check Repeat:
    if (--msgRepeat <= 0) {
      msgRepeat = STANDARDREPEAT;
      resetMsgVars();
      stockMsgTracker = 0;
      loadStockMsg();
      return;
    }
    else { pacmanStart = PACMANSTDSTART; }
  }

  //Add new column data
  if ((pacmanStart+BLINKYSTART >= 0) && (pacmanStart+BLINKYSTART < 7)) { nextCol = blinky[(pacmanStart + BLINKYSTART)]; }
  else { nextCol = 0; }
/*  
  2  4  6  8  +6)%8 || +2)%8
  10 12 14 16 +4)%8
  18 20 22 24  )%8
*/  

/* 1 2 3 4
   5 6 7 8
   9 10 11 12
   
   +3)%4 || +1)%4
   +2)%4
   )%4
*/

  //Redraw animation data as needed
  if (((pacmanStart+3) % 4 == 0) || ((pacmanStart+1) % 4 == 0)) {
    for (uint8_t i = 0; i<7; i++) {
      if (pacmanStart-7+i >= MSGCUSTOMARRAYLEN) { break; }
      if ((pacmanStart-6+i >= 0) && (pacmanStart-6-i < MSGCUSTOMARRAYLEN)) { buffer[pacmanStart-7+i] = pacman1[6-i]; }
    } 
  }
  else if ((pacmanStart+2) % 4 == 0)  {
    for (uint8_t i = 0; i<7; i++) {
      if (pacmanStart-7+i >= MSGCUSTOMARRAYLEN) { break; }
      if ((pacmanStart-6+i >= 0) && (pacmanStart-6-i < MSGCUSTOMARRAYLEN)) { buffer[pacmanStart-7+i] = pacman0[6-i]; }
    }
  }
  else if ((pacmanStart) % 4 == 0) {
    for (uint8_t i = 0; i<7; i++) {
      if (pacmanStart-7+i >= MSGCUSTOMARRAYLEN) { break; }
      if ((pacmanStart-6+i >= 0) && (pacmanStart-6-i < MSGCUSTOMARRAYLEN)) { buffer[pacmanStart-7+i] = pacman2[6-i]; }
    }
  } 
}

void loadStockMsg(void) {
  //Rotate stock colors
  if (++stockColor >= COLORSINPALETTE) { stockColor = 0; }
  curColor = colorsLow[stockColor];
  
  for (uint8_t i=0; i<MSGCUSTOMARRAYLEN; i++) {
    if (pgm_read_byte_near(msgPointers[stockMsgTracker] + i) == 0) { 
      msgBuffer[i] = 0; //zero terminate the string  
      break;
    }
    else { msgBuffer[i] = pgm_read_byte_near(msgPointers[stockMsgTracker] + i); }
  }
}

void resetMsgVars(void) {
  //Reset all variables
  msgIdx = 0; //Which letter are we on?
  chrIdx = 0; //WHich column of this letter's font are we on?
  nextCol = 0; // Next column pixels (Doesn't need to be global but whatevs)
  clearIdx = 0;
  msgState = NEXTCHAR;
}

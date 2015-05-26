#include "PCD8544.h"
#include <EEPROM.h>

// Connections to LCD display
// Arduino pin 2 -> LCD breakout board serial clock out (SCLK)
// Arduino pin 3 -> LCD breakout board serial data out (DIN)
// Arduino pin 4 -> LCD breakout board Data/Command select (D/C)
// Arduino pin 5 -> LCD breakout board LCD reset (RST)
// Arduino pin 6 -> LCD breakout board LCD chip select (CS)
PCD8544 nokia = PCD8544(2, 3, 4, 6, 5); // Initializes the LCD display

// These define the pins to which one leg of the buttons should be attached.
// The other leg should be attached to the Arduino's ground.
int OffIntervalUpButton = 7;
int OffIntervalDownButton = 8;
int OnIntervalUpButton = 9;
int OnIntervalDownButton = 10;
int SprayButton = 11;
int ResetButton = 12;

// One leg of the output relay should be atached to this pin.
// The other to the Arduino's ground.
int Valve = 13;

// I don't want to lose my interval settings when the arduino loses power, so we use the Arduino's
// built in EEPROM memory cache to store settings when they are changed. These set the memory addresses
// for the off and on interval settings
int EEPROM_OFF_INTERVAL_ADDRESS = 0;
int EEPROM_ON_INTERVAL_ADDRESS = 1;

// These set how much the interval changes when you press the up and down buttons
// Values in milliseconds
long OffIntervalSteps = 300000;
long OnIntervalSteps = 1000;

// If you hold down the up or down buttons, the interval will count up or down until you let go
// This sets the pause time between changes as you hold doun the button. Value in millisaconds.
int ButtonPressWait = 500;

// In order to make the up/down buttons count continuously up or down with a pause between jumps,
// we need a timer buffer to remember how long ago the last jump was.
long OffIntervalUpBuffer;
long OffIntervalDownBuffer;
long OnIntervalUpBuffer;
long OnIntervalDownBuffer;

// These are the interval variables
long LastSpray;
long OffTime;
long OnTime;

void setup(void) {
  Serial.begin(9600);
  nokia.init();
  nokia.setContrast(50);

  // Enable this code and push program once on the first run of a new board or to move EEPROM registers.
  // Then disable and push the program to the board again for normal operation.
  //EEPROM.write(EEPROM_OFF_INTERVAL_ADDRESS, 15);
  //EEPROM.write(EEPROM_ON_INTERVAL_ADDRESS, 3);
  
  // This grabs the stored intervals from the EEPROM cache on startup
  OffTime = EEPROM.read(EEPROM_OFF_INTERVAL_ADDRESS) * 60000;
  OnTime = EEPROM.read(EEPROM_ON_INTERVAL_ADDRESS) * 1000;
  
  // Initilize last spray time buffer, set it to right now.
  LastSpray = millis();
  
  pinMode(Valve, OUTPUT);
  digitalWrite(Valve, LOW);
  
  pinMode(OffIntervalUpButton, INPUT);
  pinMode(OffIntervalDownButton, INPUT);
  pinMode(OnIntervalUpButton, INPUT);
  pinMode(OnIntervalDownButton, INPUT);
  pinMode(ResetButton, INPUT);
  pinMode(SprayButton, INPUT);
  
  // This enables the Arduino's built in pull up resistors on these pins to make
  // wiring pushbuttons much easier.
  digitalWrite(OffIntervalUpButton, HIGH);
  digitalWrite(OffIntervalDownButton, HIGH);
  digitalWrite(OnIntervalUpButton, HIGH);
  digitalWrite(OnIntervalDownButton, HIGH);
  digitalWrite(ResetButton, HIGH);
  digitalWrite(SprayButton, HIGH);
  
  // Let the user know that we're alive in here.
  nokia.setCursor(0, 0);
  nokia.print("Booting");
  nokia.display();
  delay(1000);
  nokia.clear();
}

void loop(void) {
  // Write the UI out to the display
  nokia.print("Intervals:");
  nokia.setCursor(0, 10);
  nokia.print(" Off: ");
  nokia.print(OffTime / 1000 / 60);
  nokia.print("m");
  nokia.setCursor(0, 21);
  nokia.print("  On: ");
  nokia.print(OnTime / 1000);
  nokia.print("s");
  nokia.setCursor(0, 41);
  nokia.print("Next: ");
  nokia.print(((LastSpray + OffTime) -  millis()) / 1000 / 60);
  nokia.print("m ");
  nokia.print((((LastSpray + OffTime) -  millis()) / 1000 ) % 60);
  nokia.print("s");
  nokia.display();
  nokia.clear();
  
  // Now we loop watching for pressed buttons
  
  if (digitalRead(ResetButton) == LOW) {
    LastSpray = millis();
  }
  
  if (digitalRead(SprayButton) == LOW) {
    digitalWrite(Valve, HIGH);
  }
  else {
    digitalWrite(Valve, LOW);
  }
  
  if (digitalRead(OffIntervalUpButton) == LOW) {
    OffIntervalUp();
  }
  
  if (digitalRead(OffIntervalDownButton) == LOW) {
    OffIntervalDown();
  }
  
  if (digitalRead(OnIntervalUpButton) == LOW) {
    OnIntervalUp();
  }
  
  if (digitalRead(OnIntervalDownButton) == LOW) {
    OnIntervalDown();
  }
  
  if (LastSpray + OffTime < millis()) {
    Spray();
    LastSpray = millis();
  }
}

void Spray(void) {
  digitalWrite(Valve, HIGH);
  long SprayBuffer = millis() + OnTime;
  while (millis() < SprayBuffer) {
    nokia.setCursor(18, 10);
    nokia.print("SPRAYING");
    nokia.setCursor(37, 30);
    nokia.print((SprayBuffer -  millis()) / 1000 );
    nokia.print("s");
    nokia.display();
    nokia.clear();
  }
  digitalWrite(Valve, LOW);
}

// Do the hold down for continuous change loop, and write new value to the EEPROM
void OffIntervalUp(void) {
  if (OffTime < 14400000) {
    if (OffIntervalUpBuffer + ButtonPressWait < millis()) {
      OffTime += OffIntervalSteps;
      EEPROM.write(EEPROM_OFF_INTERVAL_ADDRESS, OffTime / 60000);
      OffIntervalUpBuffer = millis();
    }
  }
}

// Do the hold down for continuous change loop, and write new value to the EEPROM
void OffIntervalDown(void) {
  if (OffTime > 300000) {
    if (OffIntervalDownBuffer + ButtonPressWait < millis()) {
      OffTime -= OffIntervalSteps;
      EEPROM.write(EEPROM_OFF_INTERVAL_ADDRESS, OffTime / 60000);
      OffIntervalDownBuffer = millis();
    }
  }
}

// Do the hold down for continuous change loop, and write new value to the EEPROM
void OnIntervalUp(void) {
  if (OnTime < 900000) {
    if (OnIntervalUpBuffer + ButtonPressWait < millis()) {
      OnTime += OnIntervalSteps;
      EEPROM.write(EEPROM_ON_INTERVAL_ADDRESS, OnTime / 1000);
      OnIntervalUpBuffer = millis();
    }
  }
}

// Do the hold down for continuous change loop, and write new value to the EEPROM
void OnIntervalDown(void) {
  if (OnTime > 1000) {
   if (OnIntervalDownBuffer + ButtonPressWait < millis()) {
      OnTime -= OnIntervalSteps;
      EEPROM.write(EEPROM_ON_INTERVAL_ADDRESS, OnTime / 1000);
      OnIntervalDownBuffer = millis();
    }
  }
}

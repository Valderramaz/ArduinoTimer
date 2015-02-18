#include "PCD8544.h"
#include <EEPROM.h>

// pin 2 - Serial clock out (SCLK)
// pin 3 - Serial data out (DIN)
// pin 4 - Data/Command select (D/C)
// pin 5 - LCD reset (RST)
// pin 6 - LCD chip select (CS)
PCD8544 nokia = PCD8544(2, 3, 4, 6, 5);

int EEPROM_OFF_INTERVAL_ADDRESS = 0;
int EEPROM_ON_INTERVAL_ADDRESS = 1;

long OffIntervalSteps = 300000;
long OnIntervalSteps = 1000;
int ButtonPressWait = 500;

long OffIntervalUpBuffer;
long OffIntervalDownBuffer;
long OnIntervalUpBuffer;
long OnIntervalDownBuffer;

int OffIntervalUpButton = 7;
int OffIntervalDownButton = 8;
int OnIntervalUpButton = 9;
int OnIntervalDownButton = 10;
int SprayButton = 11;
int ResetButton = 12;
int Valve = 13;

long LastSpray;
long OffTime;
long OnTime;

void setup(void) {
  Serial.begin(9600);
  nokia.init();
  nokia.setContrast(50);

  //Enable this code on the first run of a new board or to move EEPROM registers
  //
  //EEPROM.write(EEPROM_OFF_INTERVAL_ADDRESS, 15);
  //EEPROM.write(EEPROM_ON_INTERVAL_ADDRESS, 3);
  
  OffTime = EEPROM.read(EEPROM_OFF_INTERVAL_ADDRESS) * 60000;
  OnTime = EEPROM.read(EEPROM_ON_INTERVAL_ADDRESS) * 1000;
  
  LastSpray = 0;
  
  pinMode(Valve, OUTPUT);
  digitalWrite(Valve, LOW);
  
  pinMode(OffIntervalUpButton, INPUT);
  pinMode(OffIntervalDownButton, INPUT);
  pinMode(OnIntervalUpButton, INPUT);
  pinMode(OnIntervalDownButton, INPUT);
  pinMode(ResetButton, INPUT);
  pinMode(SprayButton, INPUT);
  
  digitalWrite(OffIntervalUpButton, HIGH);
  digitalWrite(OffIntervalDownButton, HIGH);
  digitalWrite(OnIntervalUpButton, HIGH);
  digitalWrite(OnIntervalDownButton, HIGH);
  digitalWrite(ResetButton, HIGH);
  digitalWrite(SprayButton, HIGH);
  
  LastSpray = millis();

  nokia.setCursor(0, 0);
  nokia.print("Booting");
  nokia.display();
  delay(1000);
  nokia.clear();
}

void loop(void) {
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

void OffIntervalUp(void) {
  if (OffTime < 14400000) {
    if (OffIntervalUpBuffer + ButtonPressWait < millis()) {
      OffTime += OffIntervalSteps;
      EEPROM.write(EEPROM_OFF_INTERVAL_ADDRESS, OffTime / 60000);
      OffIntervalUpBuffer = millis();
    }
  }
}

void OffIntervalDown(void) {
  if (OffTime > 300000) {
    if (OffIntervalDownBuffer + ButtonPressWait < millis()) {
      OffTime -= OffIntervalSteps;
      EEPROM.write(EEPROM_OFF_INTERVAL_ADDRESS, OffTime / 60000);
      OffIntervalDownBuffer = millis();
    }
  }
}

void OnIntervalUp(void) {
  if (OnTime < 900000) {
    if (OnIntervalUpBuffer + ButtonPressWait < millis()) {
      OnTime += OnIntervalSteps;
      EEPROM.write(EEPROM_ON_INTERVAL_ADDRESS, OnTime / 1000);
      OnIntervalUpBuffer = millis();
    }
  }
}

void OnIntervalDown(void) {
  if (OnTime > 1000) {
   if (OnIntervalDownBuffer + ButtonPressWait < millis()) {
      OnTime -= OnIntervalSteps;
      EEPROM.write(EEPROM_ON_INTERVAL_ADDRESS, OnTime / 1000);
      OnIntervalDownBuffer = millis();
    }
  }
}
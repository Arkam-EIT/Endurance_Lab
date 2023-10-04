//Final Code Endurance Lab V.3
#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>

//Defines
#define I2C_Addr 0x20                             // I2C Address of PCF8574-board: 0x20 - 0x27
#define RELAY_MODULE D4
#define LED_Red D5
#define LED_Yellow D6
#define LED_Green D7
#define BUZZER D0
#define MILLIS_PER_MINUTE 60000

LiquidCrystal_I2C lcd(0x3F, 20, 4);
//LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long timeNow;
unsigned long currentTime;
unsigned long OverallTime;
unsigned long remainingTime;
unsigned long elapsedTime;
unsigned long estimatedTime;

String cyclesPerMinute_str;
String noOfCycles_str;
int noOfCycles;
int cyclesPerMinute;

const byte rows = 4;                           // Number of Rows
const byte columns = 4;                        // Number of Columns

unsigned long singleCycleTime;
int cycleCount = 0;
int remainingCycleCount;
boolean startFlag = false;
boolean processStartFlag = false;

//Layout of the Keys on Keypad
char KeyPadLayout[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//Wiring of PCF8574-IO-Pins
byte PinsLines[rows] = { 3, 2, 1, 0};          //  ROWS Pins
byte PinsColumns[columns] = {7, 6, 5, 4};      //  COLUMNS Pins

//Initialise KeyPad
Keypad_I2C i2cKeypad( makeKeymap(KeyPadLayout), PinsLines, PinsColumns, rows, columns, I2C_Addr);

//Setup
void setup() {
  Serial.begin(9600);

  pinMode(RELAY_MODULE, OUTPUT);
  pinMode(LED_Red, OUTPUT);
  pinMode(LED_Yellow, OUTPUT);
  pinMode(LED_Green, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(RELAY_MODULE, HIGH);
  digitalWrite(LED_Red, HIGH);

  lcd.init();
  lcd.begin (20, 4);
  lcd.backlight();

  Serial.println(F("--- Begin: Check Connection ..."));
  Wire.begin(4, 5);                                        // Init I2C-Bus, GPIO4-Data, GPIO5 Clock
  Wire.beginTransmission(I2C_Addr);                        // Try to establish connection
  if (Wire.endTransmission() != 0)                         // No Connection established
  {
    Serial.print("    NO ");
  }                                                       // if (Wire.endTransmission() != 0)
  else {
    Serial.print("    ");
  }                                                       // else to if (Wire.endTransmission() != 0)

  Serial.print(F("Device found on"));
  Serial.print(F(" (0x"));
  Serial.print(I2C_Addr, HEX);
  Serial.println(F(")."));
  Serial.println(F("--- End: Check Connection"));

  i2cKeypad.begin( );                                     // Start i2cKeypad

  welcomeMsg();
  noOfCycles = getTotalCyclesFromKeypad().toInt();
  cyclesPerMinute = getCyclesPerMinuteFromKeypad().toInt();
  startProcessMsg();
}

//Main-Loop
void loop() {
  //Logic
  // Calculate the singleCycleTime
  singleCycleTime = MILLIS_PER_MINUTE / cyclesPerMinute;

  // When the user press the push button, the timer need to start.
  // But for now we've made it by default true.
  startFlag = true; // Will get these value from push button.
  if (startFlag == true) {
    currentTime = millis();
  }

  // Calculate the Estimated time for debugging Purpose.
  OverallTime = singleCycleTime * noOfCycles;
  estimatedTime = OverallTime;
  unsigned long estimatedSecs = estimatedTime / 1000;
  int et_runHours = estimatedSecs / 3600;
  int et_secsRemaining = estimatedSecs % 3600;
  int et_runMinutes = et_secsRemaining / 60;
  int et_runSeconds = et_secsRemaining % 60;
  char et_buf[14];
  sprintf(et_buf, "%02d:%02d:%02d", et_runHours, et_runMinutes, et_runSeconds);

   // Calculate the Elapsed time for debugging Purpose.
  elapsedTime = currentTime;
  unsigned long elapsedSecs = elapsedTime / 1000;
  int e_runHours = elapsedSecs / 3600;
  int e_secsRemaining = elapsedSecs % 3600;
  int e_runMinutes = e_secsRemaining / 60;
  int e_runSeconds = e_secsRemaining % 60;
  char e_buf[14];
  sprintf(e_buf, "%02d:%02d:%02d", e_runHours, e_runMinutes, e_runSeconds);

  // By default the cycle will start from 1.
  int cycle = 1;
  while (cycle <= noOfCycles && cycleCount < noOfCycles) {
    // if the millis > sinpleCycle time make the relay turn on
    if (millis() >= (timeNow + singleCycleTime))  {
      timeNow += singleCycleTime; // Update the previous time to current time
      
      // Code to Control Relay - Turn On
      digitalWrite(RELAY_MODULE, LOW);
      digitalWrite(LED_Yellow, HIGH);
      //digitalWrite(BUZZER, HIGH);

      delay(2000); // Relay Active Time

      // Update the Cycle count
      cycleCount++;
      remainingCycleCount = noOfCycles - cycleCount;
    }

    // Increment the cycle Count
    cycle++;

    // Turn Off the Relay
    digitalWrite(RELAY_MODULE, HIGH);
    digitalWrite(LED_Yellow, LOW);
    //digitalWrite(BUZZER, LOW);

    delay(singleCycleTime);

    serialLog();
    lcdLog();
  }

  endMsg();
}

void serialLog() {
  //Logs in Serial Monitor for testing
  Serial.print("Total No. of Cycles : ");
  Serial.println(noOfCycles);
  Serial.print("Cycles Per Minute : ");
  Serial.println(cyclesPerMinute);
  Serial.print("No. of Cycles completed : ");
  Serial.println(cycleCount);
  Serial.print("No. of Cycles remaining : ");
  Serial.println(remainingCycleCount);
  //  Serial.print("Estimated time : ");
  //  Serial.println(et_buf);
  //  Serial.print("Elapsed time : ");
  //  Serial.println(e_buf);
  Serial.println();
}

void lcdLog() {
  // Logs in LCD Display
  lcd.setCursor(0, 1); // Column, Row
  lcd.print("   ENDURANCE LAB   ");

  lcd.setCursor(0, 0); // Column, Row
  lcd.print("T.CYC: ");
  lcd.setCursor(6, 0);
  lcd.print(noOfCycles);
  lcd.setCursor(13, 0);
  lcd.print("|CPM:");
  lcd.setCursor(18, 0);
  lcd.print(cyclesPerMinute);

  lcd.setCursor(0, 3); // Column, Row
  lcd.print("CYC COMPLETED: ");
  lcd.setCursor(15, 3);
  lcd.print(cycleCount);

  lcd.setCursor(0, 2); // Column, Row
  lcd.print("CYC REMAINING: ");
  lcd.setCursor(15, 2);
  lcd.print(remainingCycleCount);
}

void welcomeMsg() {
  lcd.clear();
  lcd.setCursor(0, 1); // Column, Row
  lcd.print("     WELCOME TO     ");
  lcd.setCursor(0, 0); // Column, Row
  lcd.print("    ENDURANCE LAB   ");
  lcd.setCursor(0, 2); // Column, Row
  lcd.print(" Press 'D' to start ");

  char keyRead;
  while (1) {
    keyRead = i2cKeypad.getKey();
    Serial.print("Entered Key : ");
    Serial.println(keyRead);
    if (keyRead == 'D') {
      break;
    }
  }

  delay(300);
  lcd.clear();
}

void endMsg() {
  lcd.clear();
  lcd.setCursor(0, 0); // Column, Row
  lcd.print(" Process Completed! ");
  lcd.setCursor(0, 2); // Column, Row
  lcd.print(" Press 'D' to start ");
  lcd.setCursor(0, 3); // Column, Row
  lcd.print("the new Test Cycles ");
  digitalWrite(LED_Green, HIGH);

  char keyRead;
  while (1) {
    keyRead = i2cKeypad.getKey();
    Serial.print("Entered Key : ");
    Serial.println(keyRead);
    if (keyRead == 'D') {
      digitalWrite(LED_Green, LOW);
      digitalWrite(LED_Yellow, LOW);
      ESP.restart();
      break;
    }
  }
}

String getTotalCyclesFromKeypad() {
  lcd.clear();
  int counter = 0;

  lcd.setCursor(0, 1); // Column, Row
  lcd.print("Enter Total Cycles:");
  lcd.setCursor(0, 2); // Column, Row
  lcd.print("Press * after entry");

  char keyRead;
  while (1) {
    keyRead = i2cKeypad.getKey();
    Serial.print("Entered Key : ");
    Serial.println(keyRead);

    if (keyRead == '1' || keyRead == '2' || keyRead == '3' || keyRead == '4' || keyRead == '5' || keyRead == '6' || keyRead == '7' || keyRead == '8' || keyRead == '9' || keyRead == '0') {
      lcd.setCursor(counter, 0);
      lcd.print(keyRead);
      counter++;
      noOfCycles_str.concat(keyRead);

      Serial.println ("NO OF CYLCES:");
      Serial.println (noOfCycles_str);
      Serial.println ("---------------");
    }
    else if (keyRead == '*') {
      break;
    }
  }

  delay(300);
  lcd.clear();

  return noOfCycles_str;
}

String getCyclesPerMinuteFromKeypad() {
  lcd.clear();
  int counter = 0;

  lcd.setCursor(0, 1); // Column, Row
  lcd.print("Enter CPM:");
  lcd.setCursor(0, 2); // Column, Row
  lcd.print("Press * after entry");

  char keyRead;
  while (1) {
    keyRead = i2cKeypad.getKey();
    Serial.print("Entered Key : ");
    Serial.println(keyRead);

    if (keyRead == '1' || keyRead == '2' || keyRead == '3' || keyRead == '4' || keyRead == '5' || keyRead == '6' || keyRead == '7' || keyRead == '8' || keyRead == '9' || keyRead == '0') {
      lcd.setCursor(counter, 0);
      lcd.print(keyRead);
      counter++;
      cyclesPerMinute_str.concat(keyRead);

      Serial.println ("CYLCES PER MINUTE:");
      Serial.println (cyclesPerMinute_str);
      Serial.println ("---------------");
    }
    else if (keyRead == '*') {
      break;
    }
  }

  delay(300);
  lcd.clear();

  return cyclesPerMinute_str;
}

void startProcessMsg() {
  lcd.clear();
  lcd.setCursor(0, 0); // Column, Row
  lcd.print("    Press 'D' to    ");
  lcd.setCursor(0, 3); // Column, Row
  lcd.print("  Start the process ");

  char keyRead;
  while (1) {
    keyRead = i2cKeypad.getKey();
    Serial.print("Entered Key : ");
    Serial.println(keyRead);
    if (keyRead == 'D') {
      break;
    }
  }

  delay(300);
  lcd.clear();

  lcd.setCursor(0, 1); // Column, Row
  lcd.print("Entered Details...");
  lcd.setCursor(0, 0); // Column, Row
  lcd.print("T.CYC : ");
  lcd.setCursor(8, 0); // Column, Row
  lcd.print(noOfCycles);
  lcd.setCursor(0, 3); // Column, Row
  lcd.print("CPM: ");
  lcd.setCursor(5, 3); // Column, Row
  lcd.print(cyclesPerMinute);
  delay(3000);

  lcd.clear();

  lcd.setCursor(0, 0); // Column, Row
  lcd.print("Process starting...");
  delay(3000);
  lcd.clear();


  /*
    Serial.println ("---------------");
    Serial.println ("PROCESS STARTED");
    Serial.println ("---------------");
    Serial.println ("TOTAL NUMBER OF CYCLES : ");
    Serial.println (noOfCycles_str);
    Serial.println ("CYCLES PER MINUTE : ");
    Serial.println (cyclesPerMinute_str);
    Serial.println ("---------------");
  */
}

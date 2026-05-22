#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>

// 1. LCD Pin Configuration
LiquidCrystal lcd(12, 11, A0, A1, A2, A3);

// 2. Servo and Buzzer Pin Configuration
#define SERVO_PIN 13   
#define BUZZER_PIN A4  
Servo lockServo;

// 3. Keypad Configuration (Pins 2 through 9)
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// 4. Password and Lock Settings
char masterPassword[5] = "1234"; 
char dataEntered[5];             
byte dataCounter = 0;            
boolean isLocked = false; // Starts unlocked

void setup() {
  pinMode(10, OUTPUT);
  analogWrite(10, 30); // Software contrast control
  
  lcd.begin(16, 2);
  lockServo.attach(SERVO_PIN);
  
  // Force physics calibration on boot
  lockServo.write(0); 
  isLocked = false;
  
  pinMode(BUZZER_PIN, OUTPUT);
  showReadyMessage();
}

void loop() {
  char customKey = customKeypad.getKey();
  
  if (customKey) {
    // STRICT STATE CHECK: Only allow locking if currently unlocked
    if (customKey == '#' && isLocked == false) {
      lockDoor();
      return; // Stop loop here so it doesn't instantly read the key again
    }
    
    // Only accept password entries if currently locked
    if (isLocked == true) {
      // Ignore functional letters except 'D'
      if (customKey != 'A' && customKey != 'B' && customKey != 'C' && customKey != '*' && customKey != '#' && customKey != 'D') {
        if (dataCounter < 4) {
          dataEntered[dataCounter] = customKey;
          lcd.setCursor(dataCounter, 1);
          lcd.print('*'); 
          dataCounter++;
          tone(BUZZER_PIN, 1000, 50);
        }
      }
      
      // Press 'D' to authenticate
      if (customKey == 'D') {
        if (dataCounter == 4) {
          dataEntered[dataCounter] = '\0'; 
          
          if (strcmp(dataEntered, masterPassword) == 0) {
            unlockDoor();
          } else {
            wrongPasswordAlarm();
          }
        } else {
          lcd.clear();
          lcd.print("Enter 4 Digits!");
          delay(1500);
          resetInputBuffer();
        }
      }
    }
  }
}

void lockDoor() {
  isLocked = true; // Set flag FIRST to lock down the loop
  lcd.clear();
  lcd.print("Locking Door...");
  lockServo.write(90); 
  delay(1000);
  
  tone(BUZZER_PIN, 1200, 100); delay(150);
  tone(BUZZER_PIN, 1200, 100);
  
  resetInputBuffer();
}

void unlockDoor() {
  isLocked = false; // Set flag FIRST to open up the loop
  lcd.clear();
  lcd.print("Access Granted!");
  lcd.setCursor(0, 1);
  lcd.print("Door Opened");
  
  lockServo.write(0); 
  
  tone(BUZZER_PIN, 1000, 150); delay(150);
  tone(BUZZER_PIN, 1500, 300); delay(2000);
  
  showReadyMessage();
}

void wrongPasswordAlarm() {
  lcd.clear();
  lcd.print("Wrong Password!");
  lcd.setCursor(0, 1);
  lcd.print("Access Denied");
  
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 700, 200); delay(250);
    tone(BUZZER_PIN, 400, 200); delay(250);
  }
  
  resetInputBuffer();
}

void resetInputBuffer() {
  dataCounter = 0;
  memset(dataEntered, 0, sizeof(dataEntered)); 
  if (isLocked) {
    lcd.clear();
    lcd.print("Enter Password:");
  } else {
    showReadyMessage();
  }
}

void showReadyMessage() {
  lcd.clear();
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Press # to Lock");
}
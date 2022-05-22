#include <Servo.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define defaultKeypadUnlockAttempts 2
#define openLED 2 

const byte ROWS = 4; 
const byte COLS = 3; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3}; 

const char password[] = "1001#";
int passwordLength = 5;
char currentInput[6] = "";

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
 
Servo myservo;  // create servo object to control a servo
int safeClosedPosition = 100;
int safeOpenPosition = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);  
int currentInputIndex;
int remainingKeypadAttempts;
bool safeClosed;
char openSafeReceive; 
void setup() {
  myservo.attach(10);  // attaches the servo on pin 9 to the servo object
  
  Serial.begin(9600); 

  //Serial.println("Attached pins.");
  Wire.begin();
  lcd.backlight();
  lcd.init(); 
  //Serial.println("Initialized lcd");
  //Serial.println("===== STARTED SMART SAFE FUNCTIONALITY =====");
  
  displayText("Don't open me!");
  safeClosed = true;
  openSafeReceive = '0';
  
  currentInputIndex = 0;
  remainingKeypadAttempts = defaultKeypadUnlockAttempts;

  pinMode(11, OUTPUT); // Set buzzer - pin 9 as an output
  pinMode(openLED, OUTPUT);
  //Serial.println("===== Finished setup on master =====");
}

void loop() {
          // ...for 1sec
 if(safeClosed == true){
   doKeypad();
 }
// Serial.readBytes(openSafeReceive, 1); 
 if (Serial.available() > 0) {
        openSafeReceive = Serial.read();
        if(openSafeReceive == '1'){
          openSafe();
        } else if(openSafeReceive == '0') {
          closeSafe();
        }
 } 
}

void displayText(char text[]) {
  lcd.clear();
  lcd.setCursor(0,0);
  if(strlen(text) < 20) {
    lcd.print(text);
  } 
}

void displayLetter(char letter, int row, int col){ 
  lcd.setCursor(col, row);
  lcd.print(letter);
}

void openSafe()
{
  safeClosed = false;
  myservo.write(safeOpenPosition);
  displayText("Damn it. Open!");
  remainingKeypadAttempts = defaultKeypadUnlockAttempts;
  digitalWrite(openLED, HIGH);
}
void closeSafe()
{
  safeClosed = true;
  myservo.write(safeClosedPosition);
  displayText("Don't open me!");
  remainingKeypadAttempts = defaultKeypadUnlockAttempts;
  digitalWrite(openLED, LOW);
}

void doKeypad() {
  char customKey = customKeypad.getKey();
  
  if (customKey && safeClosed){
    if(remainingKeypadAttempts > 0) {
      currentInput[currentInputIndex++] = customKey;
      currentInput[currentInputIndex] = '\0';
      displayText(currentInput);
     
      if(currentInputIndex == passwordLength) {
        if(strcmp(currentInput,password) == 0) {
          openSafe();
        }
        else {
          displayText("Wrong password");
          currentInputIndex = 0;
          strcpy(currentInput, "");
          currentInput[0]='\0';
          remainingKeypadAttempts--;

          tone(11, 1300); // Send 1KHz sound signal...
          delay(1000);        // ...for 1 sec
          noTone(11);     // Stop sound...
          delay(2000);
          
        }
      }
    } else {
      displayText("Can't use keypad");
    }
  }

}

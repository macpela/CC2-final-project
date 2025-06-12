// BOMB GAME - Modified version
// Start with key 16 instead of 3 sounds
// Code changed to 411116

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
#define CLOCK_PIN 2
#define DATA_PIN 3
const int JOYSTICK_X = A0;   
const int JOYSTICK_Y = A1;   
const int JOYSTICK_BTN = 8;  
const int MIC_PIN = A3;      
const int FAN_PIN = 7;       

// Game configuration
const String CORRECT_CODE = "411116";  // Changed from "1234" to "411116"
const unsigned long GAME_TIME = 300000;

// Game state
int currentTask = 0;
String enteredCode = "";
unsigned long gameStartTime = 0;
bool gameActive = false;
bool gameWon = false;
uint16_t lastKeyState = 0;

// Task states
bool task1Complete = false;
bool task2Complete = false;
bool task3Complete = false;
bool task4Complete = false;

// Joystick navigation
int cursorX = 8;
int cursorY = 1;
const int targetX = 12;
const int targetY = 1;

// Sound detection
int backgroundLevel = 30;
int soundCount = 0;
unsigned long lastSoundTime = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize keypad pins
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);
  digitalWrite(CLOCK_PIN, LOW);
  
  // Initialize other hardware
  pinMode(JOYSTICK_BTN, INPUT_PULLUP);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  
  // Initialize LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  
  // Calibrate microphone
  calibrateMicrophone();
  
  Serial.println("=== BOMB GAME - MODIFIED VERSION ===");
  Serial.println("Press key 16 to start!");
  Serial.println("Code is now: 411116");
  
  showStartScreen();
}

// Read keypad using polling
uint16_t readKeypad() {
  uint16_t keyState = 0;
  
  digitalWrite(CLOCK_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(CLOCK_PIN, LOW);
  delayMicroseconds(10);
  
  for (int i = 0; i < 16; i++) {
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(2);
    
    if (digitalRead(DATA_PIN)) {
      keyState |= (1 << i);
    }
    
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(2);
  }
  
  return keyState;
}

// Get which key number is pressed with CORRECT mapping
int getKeyPressed() {
  uint16_t keyState = readKeypad();
  
  if (keyState > 0 && keyState != lastKeyState) {
    lastKeyState = keyState;
    
    // Find which bit is set
    for (int i = 0; i < 16; i++) {
      if (keyState & (1 << i)) {
        // Your corrected mapping
        int keyNum = ((i + 1) % 16) + 1;
        
        Serial.print("Bit ");
        Serial.print(i);
        Serial.print(" = Key ");
        Serial.println(keyNum);
        
        return keyNum;
      }
    }
  } else if (keyState == 0) {
    lastKeyState = 0;
  }
  
  return 0;
}

void loop() {
  if (!gameActive && !gameWon) {
    checkKeyStart();  // Changed from checkVoiceStart
  } else if (gameActive) {
    // Update timer
    unsigned long elapsed = millis() - gameStartTime;
    unsigned long timeLeft = GAME_TIME > elapsed ? GAME_TIME - elapsed : 0;
    
    if (timeLeft == 0) {
      gameOver();
      return;
    }
    
    updateTimer(timeLeft);
    
    // Run current task
    switch (currentTask) {
      case 0: runTask1(); break;
      case 1: runTask2(); break;
      case 2: runTask3(); break;
      case 3: runTask4(); break;
    }
  }
}

// Microphone calibration
void calibrateMicrophone() {
  lcd.clear();
  lcd.print("Calibrating mic");
  
  int avgBackground = 0;
  for (int s = 0; s < 10; s++) {
    int maxVal = 0;
    int minVal = 1023;
    
    for (int i = 0; i < 100; i++) {
      int val = analogRead(MIC_PIN);
      if (val > maxVal) maxVal = val;
      if (val < minVal) minVal = val;
      delayMicroseconds(200);
    }
    
    avgBackground += (maxVal - minVal);
    delay(50);
  }
  
  backgroundLevel = (avgBackground / 10) + 10;
  Serial.print("Background level: ");
  Serial.println(backgroundLevel);
  
  delay(500);
}

// Sound detection
bool detectSound() {
  int maxVal = 0;
  int minVal = 1023;
  
  for (int i = 0; i < 100; i++) {
    int val = analogRead(MIC_PIN);
    if (val > maxVal) maxVal = val;
    if (val < minVal) minVal = val;
    delayMicroseconds(200);
  }
  
  int peakToPeak = maxVal - minVal;
  
  if (peakToPeak > backgroundLevel && peakToPeak > 15) {
    Serial.print("Sound detected! Level: ");
    Serial.println(peakToPeak);
    return true;
  }
  
  return false;
}

// Start screen
void showStartScreen() {
  lcd.clear();
  lcd.print("BOMB ARMED!");
  lcd.setCursor(0, 1);
  lcd.print("Press key 16!");  // Changed from "Make 3 sounds!"
}

// Key activation to start - NEW FUNCTION
void checkKeyStart() {
  int key = getKeyPressed();
  
  if (key == 16) {  // Only start when key 16 is pressed
    Serial.println("Key 16 pressed - Starting game!");
    startGame();
  }
}

// Start game
void startGame() {
  gameActive = true;
  gameStartTime = millis();
  currentTask = 0;
  
  lcd.clear();
  lcd.print("DEFUSE THE BOMB!");
  delay(1500);
  
  showTask1();
}

// Task 1: Code entry
void showTask1() {
  lcd.clear();
  lcd.print("Task 1: Code");
  lcd.setCursor(0, 1);
  lcd.print("Enter: ______");  // 6 underscores for 6-digit code
  enteredCode = "";
  
  Serial.println("\nTask 1: Enter code 411116");
}

void runTask1() {
  int key = getKeyPressed();
  
  if (key > 0) {
    enteredCode += String(key);
    
    // Display position adjusted for 6-digit code
    lcd.setCursor(7 + enteredCode.length() - 1, 1);
    
    // Show the key number (handle 2-digit numbers)
    if (key < 10) {
      lcd.print(key);
    } else {
      // For keys 10-16, show as hex or compress
      if (key == 16) {
        lcd.print("G");  // Use G for 16
      } else {
        lcd.print(key - 10);  // 10=0, 11=1, etc.
      }
    }
    
    Serial.print("Key ");
    Serial.print(key);
    Serial.print(" added. Code: ");
    Serial.println(enteredCode);
    
    if (enteredCode.length() >= 6) {  // Changed from 4 to 6
      delay(500);
      
      lcd.clear();
      lcd.print("You entered:");
      lcd.setCursor(0, 1);
      // Show abbreviated version on LCD
      for (int i = 0; i < enteredCode.length(); i++) {
        int digit = enteredCode.substring(i, i+1).toInt();
        if (digit == 0) digit = enteredCode.substring(i, i+2).toInt();
        
        if (digit < 10) {
          lcd.print(digit);
        } else if (digit == 16) {
          lcd.print("G");
        } else {
          lcd.print(digit - 10);
        }
      }
      
      Serial.print("Full code entered: ");
      Serial.println(enteredCode);
      
      delay(2000);
      
      if (enteredCode == CORRECT_CODE) {
        lcd.clear();
        lcd.print("CODE CORRECT!");
        task1Complete = true;
        currentTask = 1;
        delay(1500);
        showTask2();
      } else {
        lcd.clear();
        lcd.print("WRONG CODE!");
        delay(1500);
        showTask1();
      }
    }
  }
}

// Task 2: Joystick navigation
void showTask2() {
  lcd.clear();
  lcd.print("Task 2: Navigate");
  cursorX = 0;
  cursorY = 1;
  
  Serial.println("\nTask 2: Navigate to X");
}

void runTask2() {
  int xVal = analogRead(JOYSTICK_X);
  int yVal = analogRead(JOYSTICK_Y);
  
  lcd.setCursor(cursorX, cursorY);
  lcd.print(" ");
  
  if (xVal < 400 && cursorX > 0) cursorX--;
  else if (xVal > 600 && cursorX < 15) cursorX++;
  
  if (yVal < 400 && cursorY > 0) cursorY = 0;
  else if (yVal > 600 && cursorY < 1) cursorY = 1;
  
  lcd.setCursor(targetX, targetY);
  lcd.print("X");
  
  lcd.setCursor(cursorX, cursorY);
  lcd.print(">");
  
  if (cursorX == targetX && cursorY == targetY) {
    lcd.clear();
    lcd.print("TARGET REACHED!");
    task2Complete = true;
    currentTask = 2;
    delay(1500);
    showTask3();
  }
  
  delay(150);
}

// Task 3: Sound detection
void showTask3() {
  lcd.clear();
  lcd.print("Task 3: Sound");
  lcd.setCursor(0, 1);
  lcd.print("Make 4 sounds: 0");
  soundCount = 0;
  
  Serial.println("\nTask 3: Make 4 sounds");
}

void runTask3() {
  if (detectSound() && millis() - lastSoundTime > 600) {
    soundCount++;
    lastSoundTime = millis();
    
    lcd.setCursor(15, 1);
    lcd.print(soundCount);
    
    if (soundCount >= 4) {
      lcd.clear();
      lcd.print("SOUNDS COMPLETE!");
      task3Complete = true;
      currentTask = 3;
      delay(1500);
      showTask4();
    }
  }
}

// Task 4: Final button
void showTask4() {
  lcd.clear();
  lcd.print("Task 4: Cooling");
  lcd.setCursor(0, 1);
  lcd.print("Press joystick!");
}

void runTask4() {
  if (digitalRead(JOYSTICK_BTN) == LOW) {
    digitalWrite(FAN_PIN, HIGH);
    
    lcd.clear();
    lcd.print("BOMB DEFUSED!");
    lcd.setCursor(0, 1);
    lcd.print("YOU WIN!");
    
    Serial.println("\n*** BOMB DEFUSED! ***");
    
    gameActive = false;
    gameWon = true;
    
    delay(3000);
    digitalWrite(FAN_PIN, LOW);
    
    delay(2000);
    resetGame();
  }
}

// Timer update
void updateTimer(unsigned long timeLeft) {
  int minutes = timeLeft / 60000;
  int seconds = (timeLeft % 60000) / 1000;
  
  lcd.setCursor(12, 0);
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

// Game over
void gameOver() {
  lcd.clear();
  lcd.print("TIME'S UP!");
  lcd.setCursor(0, 1);
  lcd.print("BOOM!");
  
  gameActive = false;
  
  delay(5000);
  resetGame();
}

// Reset game
void resetGame() {
  currentTask = 0;
  enteredCode = "";
  gameActive = false;
  gameWon = false;
  task1Complete = false;
  task2Complete = false;
  task3Complete = false;
  task4Complete = false;
  soundCount = 0;
  cursorX = 0;
  cursorY = 1;
  lastKeyState = 0;
  
  calibrateMicrophone();
  showStartScreen();
}
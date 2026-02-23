#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LDR_D0 27

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---- Timing ----
const unsigned long DOT_MAX    = 500;
const unsigned long LETTER_GAP = 750;
const unsigned long WORD_GAP   = 1800;   // 🔥 space between words
const unsigned long DEBOUNCE   = 30;

unsigned long signalStart = 0;
unsigned long gapStart = 0;
unsigned long lastChange = 0;

int lastState = HIGH;

// ---- Buffers ----
char morse[10];
int morseIndex = 0;

// ---- LCD cursor ----
int lcdCol = 0;
int lcdRow = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LDR_D0, INPUT);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Morse Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  int state = digitalRead(LDR_D0);
  unsigned long now = millis();

  if (state != lastState && (now - lastChange) < DEBOUNCE) return;

  if (state != lastState) {
    lastChange = now;

    // Light OFF → pulse finished
    if (lastState == LOW && state == HIGH) {
      unsigned long duration = now - signalStart;

      if (morseIndex < 9) {
        morse[morseIndex++] = (duration <= DOT_MAX) ? '.' : '-';
        morse[morseIndex] = '\0';
      }

      gapStart = now;
    }

    // Light ON
    if (lastState == HIGH && state == LOW) {
      signalStart = now;
    }

    lastState = state;
  }

  // -------- END OF LETTER --------
  if (morseIndex > 0 && state == HIGH && (now - gapStart) > LETTER_GAP) {

    char letter = decodeMorse(morse);

    lcd.setCursor(lcdCol, lcdRow);
    lcd.print(letter);
    Serial.print(letter);

    lcdCol++;

    // Line wrap
    if (lcdCol >= 16) {
      lcdCol = 0;
      lcdRow++;
      if (lcdRow >= 2) {
        lcd.clear();
        lcdRow = 0;
      }
    }

    morseIndex = 0;
    gapStart = now;
  }

  // -------- WORD GAP (SPACE) --------
  if (state == HIGH && morseIndex == 0 && gapStart > 0 && (now - gapStart) > WORD_GAP) {

    lcd.setCursor(lcdCol, lcdRow);
    lcd.print(" ");
    Serial.print(" ");

    lcdCol++;

    if (lcdCol >= 16) {
      lcdCol = 0;
      lcdRow++;
      if (lcdRow >= 2) {
        lcd.clear();
        lcdRow = 0;
      }
    }

    gapStart = 0;   // prevent multiple spaces
  }
}

// ---- MORSE DECODER ----
char decodeMorse(const char *code) {
  if (!strcmp(code, ".-"))   return 'A';
  if (!strcmp(code, "-...")) return 'B';
  if (!strcmp(code, "-.-.")) return 'C';
  if (!strcmp(code, "-.."))  return 'D';
  if (!strcmp(code, "."))    return 'E';
  if (!strcmp(code, "..-.")) return 'F';
  if (!strcmp(code, "--."))  return 'G';
  if (!strcmp(code, "....")) return 'H';
  if (!strcmp(code, ".."))   return 'I';
  if (!strcmp(code, ".---")) return 'J';
  if (!strcmp(code, "-.-"))  return 'K';
  if (!strcmp(code, ".-..")) return 'L';
  if (!strcmp(code, "--"))   return 'M';
  if (!strcmp(code, "-."))   return 'N';
  if (!strcmp(code, "---"))  return 'O';
  if (!strcmp(code, ".--.")) return 'P';
  if (!strcmp(code, "--.-")) return 'Q';
  if (!strcmp(code, ".-."))  return 'R';
  if (!strcmp(code, "..."))  return 'S';
  if (!strcmp(code, "-"))    return 'T';
  if (!strcmp(code, "..-"))  return 'U';
  if (!strcmp(code, "...-")) return 'V';
  if (!strcmp(code, ".--"))  return 'W';
  if (!strcmp(code, "-..-")) return 'X';
  if (!strcmp(code, "-.--")) return 'Y';
  if (!strcmp(code, "--..")) return 'Z';
  return '?';
}
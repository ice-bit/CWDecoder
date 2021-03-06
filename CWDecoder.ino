#include <LiquidCrystal.h>
#define BUZZER_PIN 2 // Buzzer pin
#define KEY_PIN 4 // Straight key pin
#define RESET_PIN 3 // Reset push button
#define DELIM "*" // Delimiter to split a word
#define LCD_NUM_ROWS 2
#define LCD_NUM_COLS 16


typedef struct Button {
  const uint8_t pin = KEY_PIN;
  const uint8_t debounce = 10;
  uint64_t counter = 0; // How long button has been pressed
  // {prev,current}State variables are used to determine state switch
  uint8_t prevState = LOW; 
  uint8_t currentState;
} Button_t;

const uint8_t rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // Setup display
Button_t key; // create a new button instance
const uint8_t shortPress = 50; // ms for a short press(dit)
const uint8_t longPress = 200; // ms for a long press(dah)
const uint16_t wpm = 500; // words-per-minutes(500 ~ 20 wpm, counter isn't accurate FIXME:)
uint64_t timeLast = 0; // ms since last word
bool alreadyTranslated = true; // To prevent adding sampling
int16_t pos = -1; // default value means empty buffer
char buf[10]; // A buffer to store data for conversion
uint8_t currentColNum; // Current column number
uint8_t currentRowNum; // Current row number

// Internal function
void buttonEvent(uint8_t type_of_event);
void translate();
char converter(const char *ch);

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(key.pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RESET_PIN, INPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  tone(BUZZER_PIN, 55000, 200); // beep for 200 ms at start
}

void loop() {

  // If reset button is pressed, reset lcd
  if(!digitalRead(RESET_PIN)) {
    lcd.clear();
    currentColNum = currentRowNum = 0;
  }

  // Read key current state
  key.currentState = !digitalRead(key.pin);

  // Check if state is changed
  if(key.currentState != key.prevState) {
    // Dirty hack for mechanical deboucing(a Schmitt trigger would be a better solution)
    delay(key.debounce);
    // In case of a bounce, update currentStatus
    key.currentState = !digitalRead(key.pin);
    if(key.currentState == HIGH) {
      // Start tone generator and turn on the LED
      tone(BUZZER_PIN, 55000);
      digitalWrite(LED_BUILTIN, HIGH);
      // Record when button was pressed
      key.counter = millis();
    } else if(key.currentState == LOW) {
      // Button is no longer pressed, get how long it was 
      // in previous state(ie how long it was pressed)
      uint64_t currentTime = millis();
      // If active key time is at least equal to short press time and lower than long press time 
      // then handle a short press event
      if(((currentTime - key.counter) >= shortPress) && (currentTime - key.counter) < longPress)
        buttonEvent(0); // Handle a short press
      else if((currentTime - key.counter) >= longPress) // Otherwise
        buttonEvent(1); // Handle a long press

      // Update time since last word
      timeLast = millis();
      // Disable tone generator and turn dff the LED
      noTone(BUZZER_PIN);
      digitalWrite(LED_BUILTIN, LOW);
    }
    // Then update previous state for the next iteration
    key.prevState = key.currentState;
  }

  /* if endTime-starTime (time since last keystroke)
  is greater than "word-per-minutes" time and 
  buffer is empty, translate the last word.
  Buffer must not be empty to prevent sampling
  when there are no data at all.
  Also, in order to prevent multiple sampling 
  we need a boolean flag */
  if(((millis() - timeLast) > wpm) && (pos != -1) && !alreadyTranslated) {
    translate();
    alreadyTranslated = true; // Set flag to true(i.e. do not add another marker)
  }

  /* Even if buffer overflows are very uncommon
    (since we empty the buffer after each word),
    a dumb user could write n characters one 
    behind the other(where n = sizeof(buf)). */
  if(pos >= 10) {
    memset(buf, 0, sizeof(buf));
    pos = -1;
  }
}

void buttonEvent(uint8_t type_of_event) {
  if(type_of_event == 0) { // short press
    buf[++pos] = '.'; // Add a 'dit' to the buffer
    alreadyTranslated = false; // Set flag to false(i.e. sample the word at next iteration)
  } else if(type_of_event == 1) { // long press
    buf[++pos] = '-'; // Add a 'dah' to the buffer
    alreadyTranslated = false; // Set flag to false(i.e. sample the word at next iteration)
  }
}

void translate() {
  // Translate word
  char translatedWord = converter(buf);

  if(currentColNum == 16) { // if we reached the end of current line
    if(currentRowNum == 1) { // and If we already are on second line
      lcd.clear(); // Clear the screen and set cursor to (0,0)
      currentColNum = currentRowNum = 0; // Reset counters
    } else { // otherwise just go to the next row
      currentColNum = 0; // Reset column pos
      lcd.setCursor(currentColNum, ++currentRowNum); // Set cursor at next line
    }
  }
  // print character to the screen
  lcd.print(translatedWord);
  // Go to the next column
  currentColNum++;
  // Empty buffer
  memset(buf, 0, sizeof(buf));
  pos = -1;
}

char converter(const char *ch) {
    // Letters
    if(!strcmp(ch, ".-")) { return 'a'; }
    else if(!strcmp(ch, "-...")) { return 'b'; }
    else if(!strcmp(ch, "-.-.")) { return 'c'; }
    else if(!strcmp(ch, "-..")) { return 'd'; }
    else if(!strcmp(ch, ".")) { return 'e'; }
    else if(!strcmp(ch, "..-.")) { return 'f'; }
    else if(!strcmp(ch, "--.")) { return 'g'; }
    else if(!strcmp(ch, "....")) { return 'h'; }
    else if(!strcmp(ch, "..")) { return 'i'; }
    else if(!strcmp(ch, ".---")) { return 'j'; }
    else if(!strcmp(ch, "-.-")) { return 'k'; }
    else if(!strcmp(ch, ".-..")) { return 'l'; }
    else if(!strcmp(ch, "--")) { return 'm'; }
    else if(!strcmp(ch, "-.")) { return 'n'; }
    else if(!strcmp(ch, "---")) { return 'o'; }
    else if(!strcmp(ch, ".--.")) { return 'p'; }
    else if(!strcmp(ch, "--.-")) { return 'q'; }
    else if(!strcmp(ch, ".-.")) { return 'r'; }
    else if(!strcmp(ch, "...")) { return 's'; }
    else if(!strcmp(ch, "-")) { return 't'; }
    else if(!strcmp(ch, "..-")) { return 'u'; }
    else if(!strcmp(ch, "...-")) { return 'v'; }
    else if(!strcmp(ch, ".--")) { return 'w'; }
    else if(!strcmp(ch, "-..-")) { return 'x'; }
    else if(!strcmp(ch, "-.--")) { return 'y'; }
    else if(!strcmp(ch, "--..")) { return 'z'; }
    // Numerals
    else if(!strcmp(ch, "-----")) { return '0'; }
    else if(!strcmp(ch, ".----")) { return '1'; }
    else if(!strcmp(ch, "..---")) { return '2'; }
    else if(!strcmp(ch, "...--")) { return '3'; }
    else if(!strcmp(ch, "....-")) { return '4'; }
    else if(!strcmp(ch, ".....")) { return '5'; }
    else if(!strcmp(ch, "-....")) { return '6'; }
    else if(!strcmp(ch, "--...")) { return '7'; }
    else if(!strcmp(ch, "---..")) { return '8'; }
    else if(!strcmp(ch, "----.")) { return '9'; }
}

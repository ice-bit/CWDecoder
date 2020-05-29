#include <string.h>
#define BUZZER_PIN  A5 // Buzzer pin
#define KEY_PIN 3 // Straight key pin

const uint8_t shortPress = 50; // ms for a short press(dit)
const uint8_t longPress = 200; // ms for a long press(dah)
const uint16_t wpm = 700; // words-per-minutes
uint64_t timeLast = 0; // ms since last word
int16_t pos = -1; // default value means empty buffer
char buf[256]; // A buffer to store data for conversion

typedef struct Button {
  const uint8_t pin = KEY_PIN;
  const uint8_t debounce = 10;
  uint64_t counter = 0; // How long button has been pressed
  // {prev,current}State variables are used to determine state switch
  uint8_t prevState = LOW; 
  uint8_t currentState;
} Button_t;

// create a new button instance
Button_t key;

// Internal function
void buttonEvent(uint8_t type_of_event);

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(key.pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  tone(BUZZER_PIN, 55000, 200);
  Serial.begin(115200);
  Serial.println("Powered on");
}

void loop() {
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
      // Disable tone generator and turn dff the LED
      noTone(BUZZER_PIN);
      digitalWrite(LED_BUILTIN, LOW);
    }
    // Then update previous state for the next iteration
    key.prevState = key.currentState;
    Serial.println(buf);
  }

  // Prevent buffer overflow
  if(pos >= 255) {
    memset(buf, 0, sizeof(buf));
    pos = -1;
  }
}

void buttonEvent(uint8_t type_of_event) {
  if(type_of_event == 0) { // short press
    /* if endTime-starTime (since last keystroke)
      is greater than word-per-minutes time and 
      buffer is empty, add a marker between words.
      Buffer must not be empty to prevent adding 
      marker char at the first input. */
    if(((millis() - timeLast) > wpm) && (pos != -1))
      buf[++pos] = ' ';

    buf[++pos] = '.';
    timeLast = millis(); // Update time since last word
  } else if(type_of_event == 1) { // long press
    if(((millis() - timeLast) > wpm) && (pos != -1))
      buf[++pos] = ' ';

    buf[++pos] = '-';
    timeLast = millis(); // Update time since last word
  }
}
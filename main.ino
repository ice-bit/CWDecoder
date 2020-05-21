const uint8_t buzzer_pin = A5; // Buzzer pin
const uint8_t key_pin = 3; // Straight key pin

const unsigned long shortPress = 50;
const unsigned long longPress = 200;

typedef struct Button {
  const uint8_t pin = key_pin;
  const uint8_t debounce = 10;
  uint16_t counter = 0; // How long button has been pressed
  // {prev,current}State variables are used to determine state switch
  uint8_t prevState = LOW; 
  uint8_t currentState;
} Button_t;

// create a new button instance
Button_t key;

void setup() {
  pinMode(buzzer_pin, OUTPUT);
  pinMode(key.pin, INPUT);

  Serial.begin(9600);
}

void loop() {
  key.currentState = !digitalRead(key.pin); // Read key current state

  // Check if it's changed from the last state
  if(key.currentState != key.prevState) {
    // Dirty hack for mechanical deboucing(a Schmitt trigger would be a better solution)
    delay(key.debounce);
    // In case of a bounce, update currentStatus
    key.currentState = !digitalRead(key.pin);
    if(key.currentState == HIGH) {
      // Start tone generator
      tone(buzzer_pin, 55000);
      // Record when button was pressed
      key.counter = millis();
    } else if(key.currentState == LOW) {
      // Button is no longer pressed, get how long it was 
      // in previous state(ie how long it was pressed)
      unsigned long currentMillis = millis();
      // If active key time is at least equal to short press time and lower than long press time 
      // then handle a short press event
      if(((currentMillis - key.counter) >= shortPress) && (currentMillis - key.counter) < longPress)
        shortPressEvent(); // Handle a short press
      else if((currentMillis - key.counter) >= longPress) // Otherwise
        longPressEvent(); // Handle a long press
      // Disable tone generator
      noTone(buzzer_pin);
    }
    // Then update previous state for the next iteration
    key.prevState = key.currentState;
  }
}

void shortPressEvent() {
  Serial.print(".");
}

void longPressEvent() {
  Serial.print("-");
}
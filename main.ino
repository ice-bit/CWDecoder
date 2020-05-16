// GPIO setup
const short input_key = 3; // Straight key pin
const short buzzer_pin = A5; // Buzzer output pin

// Dit-dah vars
short key_previous_state = LOW; // Previous state of the key
const unsigned short dah_duration = 200; // How long a 'dah' should last(in ms).
unsigned long key_long_press_ms; // How long the key has been pressed(long press)
bool dah_state = false; // True if is a dah, false otherwise
const short delay_in = 2; // Time between two samplings(in ms).
unsigned long prev_key_ms; // Latest reading's time
unsigned long key_press_ms; // How long the key was pressed(short press)
unsigned long ticks; // Number of milliseconds passed since the program started 

void read_state() {
  // Check wether enough time has passed before making another sampling
  if((ticks - prev_key_ms) > delay_in) { 
    // Retrive key state
    int key_state = !digitalRead(input_key);

  // Check if key is pressed, previous state was low and no other
  // measurement was already running
    if(key_state == HIGH && key_previous_state == LOW && !dah_state) {
      key_long_press_ms = ticks;
      key_previous_state = HIGH; // Update previous state for next iteration
    }

    // Check how long key has been pressed 
    key_press_ms = ticks - key_long_press_ms;

  // Check if key is pressed, dah state wasn't enabled and check if 
  // press duration is greather or equal to the default dah duration
    if(key_state == HIGH && !dah_state && key_press_ms >= dah_duration) {
      dah_state = true; 
      Serial.print("-");
      tone(buzzer_pin, 55000, 300);
      return;
    } 
    if(key_state == LOW && key_previous_state == HIGH && key_press_ms < dah_duration) { // If key is released(and it was pressed) return at the original state
      key_previous_state = LOW;
      dah_state = false;
      Serial.print(".");
      tone(buzzer_pin, 55000, 100);
    }
    // Update time for next iteration
    prev_key_ms = ticks;
  }
}

void setup() {
  Serial.begin(9600); 

  pinMode(input_key, INPUT); // Set key as an input
  pinMode(buzzer_pin, OUTPUT); // Set buzzer as an output
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Ready");
}

void loop() {
  ticks = millis(); // Retrieve current time
  read_state();
}
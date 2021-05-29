
#define BOARD_BUTTON PA5
#define MODULE_5V_EN PB6
#define MODULE_PIRA_5V PA8

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status
bool poweron = false;
void setup() {
  pinMode(MODULE_5V_EN, OUTPUT);
  digitalWrite(MODULE_5V_EN, LOW);
  pinMode(MODULE_PIRA_5V, OUTPUT);
  digitalWrite(MODULE_PIRA_5V, LOW);
  // initialize the pushbutton pin as an input:
  pinMode(BOARD_BUTTON, INPUT); 
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(BOARD_BUTTON);
  
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    if (!poweron) {
      poweron = true;
      digitalWrite(MODULE_5V_EN, HIGH);
      digitalWrite(MODULE_PIRA_5V, HIGH);
    }
  }
  
  delay(100);
}

#include "Button.h"

Button::Button(int pin, String buttonName) {
	_pin = pin;
	_buttonName = buttonName;

  pinMode(pin, INPUT);
}

bool Button::button_pressed() {
  
  int currentState = digitalRead(_pin);

  if (currentState) {
    while (true) {
      currentState = digitalRead(_pin);
      if (currentState ^ 1) {
        Serial.println(_buttonName+" pressed");
        return true;
      }  
    }
  }
  return false;
}
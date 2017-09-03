#ifndef BUTTON_H
#define BUTTON_H
#include "Arduino.h"

class Button {
public:
	Button(int pin, String buttonName);
	bool button_pressed();
private:
	int _pin;
	String _buttonName;
};
#endif

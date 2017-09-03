#ifndef BUTTONPAIR_H
#define BUTTONPAIR_H
#include "Arduino.h"
#include "Button.h"

class ButtonPair {
public:
	ButtonPair(int p1, int p2, String b1Name, String b2Name, int maxVal, String buttonPairName);
	int button_pressed(bool op);
	void setVals(const int val, const int maxVal);
private:
	int increment();
	int decrement();
	bool check_lower_bound();
	bool check_upper_bound();
	int _val;
	int _p1;
	int _p2;
	int _maxVal;
	String _buttonPairName;

	Button _incButton;
	Button _decButton;
};
#endif
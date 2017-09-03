#include "ButtonPair.h"
#include "Button.h"

ButtonPair::ButtonPair(int p1, int p2, String b1Name, String b2Name, int maxVal, String buttonPairName) :
	_incButton(p1, b1Name), _decButton(p2, b2Name) {
	
	_p1 = p1;
	_p2 = p2;
	_maxVal = maxVal;
	_buttonPairName = buttonPairName;
}

bool ButtonPair::check_lower_bound() {
	return _val > 0;
}

bool ButtonPair::check_upper_bound() {
	return _val < _maxVal;
} 

// check if in range
int ButtonPair::increment() {
	if (check_upper_bound())
		_val++;
	return _val;
}

int ButtonPair::decrement() {
	if (check_lower_bound())
		_val--;
	return _val;
}

void ButtonPair::setVals(const int val, const int maxVal) {
	_val = val;
	_maxVal = maxVal;
}

int ButtonPair::button_pressed(bool op) {
	bool b1 = _incButton.button_pressed();
	bool b2 = _decButton.button_pressed();

	if (b1) {
		if (op)
			_val = decrement();
		else
			_val = increment();
	}
	else if (b2) {
		if (op)
			_val = increment();
		else
			_val = decrement();
	}
	return _val;
}
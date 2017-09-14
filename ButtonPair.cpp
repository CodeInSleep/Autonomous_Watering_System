#include "ButtonPair.h"
#include "Button.h"

ButtonPair::ButtonPair(int p1, int p2, String b1Name, String b2Name, String buttonPairName) :
	_incButton(p1, b1Name), _decButton(p2, b2Name) {
	
	_p1 = p1;
	_p2 = p2;
	_buttonPairName = buttonPairName;
}

// minVal is the lowest possible value of the ButtonPair
bool ButtonPair::check_lower_bound() {
	return _val > _minVal;
}

// maxVal is the hight possible value of the ButtonPair
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

void ButtonPair::setVals(const int val, const int minVal, const int maxVal) {
	_val = val;
	_minVal = minVal;
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
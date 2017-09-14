#ifndef LCDDISPLAY_I2C_H
#define LCDDISPLAY_I2C_H
#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "Button.h"
#include "ButtonPair.h"

//#define COUNTOF(x) (sizeof(x)/sizeof(*x))

class lcdDisplayI2C {
public:
	// lcdDisplay params:
	// int controlPins = [int rs, int en, int d4, int d5, int d6, int d7]
	// int buttonPins = [int up, int down, int left, int right, int enter]
	lcdDisplayI2C(int addr, int col, int row, int up, int down, int left, int right, int enter, int activationPin);
	// ~lcdDisplay();
	void update_time_setting_line();
	void move_tick();

	void show_main_page();
	void show_time_setting_page();
	// return what page the lcdDisplay is currently on
	int get_page_num();
	// void display();
	// struct timeDigit {
	// 	int _val;
	// 	String _name;
	// };
	// class displayDigit {
	// public:
	// 	displayDigit(const int val, const int String);
	// 	void getVal();
	// 	void setVal(const int val);
	// 	String name();
	// private:
	// 	int _val;
	// 	String _name;
	// };
	
	//static unsigned long get_millis(String timeString);

private:
	LiquidCrystal_I2C _lcd_i2c;

	ButtonPair _upDown;
	ButtonPair _leftRight;
	Button _enter;
	int _pageNum;

	// String _bigInterval;
	// String _nextStartTime;
	// String _smallInterval;
	// String _numberOfTimes;
	// String _openValveDuration;
	String _timeSettings[5];
	int _activationPin;

	// can only change in main page
	int _cursorRowPosition = 0;

	// can only change in time setting page
	int _cursorColPosition = 0;

	// timeDigit timeDigits[6];

	// main menu lines
	static const char* MENU_LINE1;
	static const char* MENU_LINE2;
	// const String _timeDigitNames[6] = {"period_hour", "period_minute", "period_second", "next_up_hour", "next_up_minute", "next_up_second"};
	unsigned long _timeSettingsInNum[5];
	unsigned long previousMillis = 0;

	bool inPeriod = false;
	bool enabled = true;


	bool enter_pressed();
	//String time_to_string(const int hour, const int min, const int sec);
	//String time_to_string(timeDigit* time);
	void move_cursor(const int col, const int row);
	String zero_pad(const int time_val);
	void init_time_digits();
	void update_time_settings();
	void render_main_page(bool reverse);
	void render_time_setting_page();

	unsigned long get_millis(String timeString);
};
#endif

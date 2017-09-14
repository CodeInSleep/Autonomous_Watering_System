#include "lcdDisplayI2C.h"
#include <Time.h>
//#include <TimeLib.h>

static const char TICK = '-';
static const char EMP = ' ';
static const int OFFSET = 3;
static const int ASCII_ZERO = 48;
static const int ASCII_FIVE = ASCII_ZERO+5;
static const int ASCII_NINE = ASCII_ZERO+9;


const char* lcdDisplayI2C::MENU_LINE1 = "Set time int";
const char* lcdDisplayI2C::MENU_LINE2 = "Set strt time";

lcdDisplayI2C::lcdDisplayI2C(int addr, int col, int row, int up, int down, int left, int right, int enter, int activationPin) 
  : _lcd_i2c(addr, col, row), _upDown(down, up, "down", "up", "upDown"), _leftRight(right, left, "left", "right", "leftRight"),
  _enter(enter, "enter") {

  _activationPin = activationPin;
  pinMode(activationPin, OUTPUT);

  // TEST period and next startTime
  _period = "00:00:10";
  _nextStartTime = "00:00:10";
  _lcd_i2c.begin();
  // init_time_digits();
  show_main_page();
}

// can only be called when on time setting page, printing out the new time digits
void lcdDisplayI2C::update_line(int lineNum) {
  _lcd_i2c.setCursor(OFFSET, lineNum);

  if (lineNum == 0)
    _lcd_i2c.print(_period);
  else if (lineNum == 1)
    _lcd_i2c.print(_nextStartTime);
}

// can only be called when on main page
void lcdDisplayI2C::move_tick(int toLine) {
   for (int i = 0; i < 2; i++) {
    _lcd_i2c.setCursor(1, i);
    if (toLine == i) {
      _lcd_i2c.write(TICK);
    }
    else
      _lcd_i2c.write(EMP);
  }
}

void lcdDisplayI2C::show_main_page() {
  render_main_page();
  update_time_settings();

  Serial.println("Showing main page");

  int newCursorRowPosition;

  // limit the up down movement of the cursor
  _upDown.setVals(_cursorRowPosition, 0, 1);

  int c;
  while (true) {
    // if enter is pressed store the time settings
    if (enter_pressed()) {
      show_time_setting_page();
      return;
    }

    newCursorRowPosition = _upDown.button_pressed(false);

    // update cursor position
    if (newCursorRowPosition != _cursorRowPosition) {
      _cursorRowPosition = newCursorRowPosition;
      move_tick(_cursorRowPosition);
    }
    
    unsigned long currentMillis = millis();
    unsigned long waitTime = interval;
    Serial.print("currentMillis: ");
    Serial.println(currentMillis);
    
    // first time
    if (!inPeriod) {
      waitTime = nextStartTimeFromNow;
    }
    
    if ((unsigned long)(currentMillis - previousMillis) >= waitTime) {
      // activate valve for 5 seconds (TODO: make activation customizable)
      Serial.println("TURN ON");
      digitalWrite(_activationPin, HIGH);
      delay(2000);
      digitalWrite(_activationPin, LOW);

      inPeriod = true;
      previousMillis = currentMillis;
    }
  }
}

void lcdDisplayI2C::show_time_setting_page() {
  render_time_setting_page();
  _leftRight.setVals(0, 0, 5);

  int newLeftRightPos;
  int newUpDownVal;
  int COLON_OFFSETS = _cursorColPosition/2;

  char* timeString = &_period[0];
  if (_cursorRowPosition == 1)
    timeString = &_nextStartTime[0];

  int currentTimeDigit = (int) timeString[_cursorColPosition+COLON_OFFSETS];

  // set updown max value for the first digit upon entering mode
  _upDown.setVals(currentTimeDigit, ASCII_ZERO, ASCII_NINE);

  while (true) {
    if (enter_pressed()) {
      inPeriod = false;
      // set previousMillis to the time exiting the time setting page
      previousMillis = millis();
      show_main_page();
    }

    newLeftRightPos = _leftRight.button_pressed(false);

    if (newLeftRightPos != _cursorColPosition) {
      _cursorColPosition = newLeftRightPos;

      COLON_OFFSETS = _cursorColPosition/2;

      // update cursor position
      move_cursor(_cursorColPosition+OFFSET+COLON_OFFSETS, _cursorRowPosition);

      // adjust the updown max value of the column 
      int newUpDownMaxVal;
      
      // if even positions (setting the tenth digit)
      if (_cursorColPosition == 0 || _cursorColPosition == 1) {
        newUpDownMaxVal = ASCII_NINE;
      } else if (_cursorColPosition % 2 == 0) {
        newUpDownMaxVal = ASCII_FIVE;
      } else {
        newUpDownMaxVal = ASCII_NINE;
      }

      currentTimeDigit = (int) timeString[_cursorColPosition+COLON_OFFSETS];
      Serial.print("currentTimeDigit: ");
      Serial.println(currentTimeDigit);
      _upDown.setVals(currentTimeDigit, ASCII_ZERO, newUpDownMaxVal);
      // continue to next iteration
      continue;
    }

    newUpDownVal = _upDown.button_pressed(true);
    
    // update timeDigits
    if (newUpDownVal != currentTimeDigit) {
      
      currentTimeDigit = newUpDownVal;
      timeString[_cursorColPosition+COLON_OFFSETS] = (char)newUpDownVal;

      // specific cases for controlling hours from exceeding 72 hours
      // if (_cursorColPosition == 0 && currentTimeDigit == ASCII_ZERO+7 && _period[OFFSET+1] > ASCII_ZERO+2)
      //   _period[OFFSET+1] = ASCII_ZERO+2;

      // if (_cursorColPosition == 1 && currentTimeDigit == ASCII_ZERO+2 && _period[OFFSET] == ASCII_ZERO+7)
      //   continue;
      
      update_line(_cursorRowPosition);

      move_cursor(_cursorColPosition+OFFSET+COLON_OFFSETS, _cursorRowPosition);
    }
    delay(50);
  }   
}

void lcdDisplayI2C::update_time_settings() {
  interval = get_millis(_period);
  nextStartTimeFromNow = lcdDisplayI2C::get_millis(_nextStartTime);
}

unsigned long lcdDisplayI2C::get_millis(String timeString) {
  unsigned long hours = (unsigned long)(timeString.substring(0, 2).toInt())*36000000;
  unsigned long mins = (unsigned long)(timeString.substring(3, 5).toInt())*60000;
  unsigned long secs = (unsigned long)(timeString.substring(6, 8).toInt())*1000;
  return hours+mins+secs;
}

void lcdDisplayI2C::render_main_page() {
  _pageNum = 1;
  _lcd_i2c.noCursor();
  _lcd_i2c.noBlink();
  _lcd_i2c.clear();
  move_cursor(1, _cursorRowPosition);
  _lcd_i2c.print(TICK);
  move_cursor(3, 0);
  _lcd_i2c.print("Set time int");
  move_cursor(3, 1);
  _lcd_i2c.print("Set strt time");
}

void lcdDisplayI2C::render_time_setting_page() {
  _cursorColPosition = 0;
  _pageNum = 2;
  _lcd_i2c.clear();

  // set the period that for the watering to be activated
  move_cursor(3, 0);
  _lcd_i2c.print(_period);

  // set time from current time to start first cycle
  move_cursor(3, 1);
  _lcd_i2c.print(_nextStartTime);

  move_cursor(3, _cursorRowPosition);
  _lcd_i2c.cursor();
  _lcd_i2c.blink();
}

int lcdDisplayI2C::get_page_num() {
  return _pageNum;
}

bool lcdDisplayI2C::enter_pressed() {
  return _enter.button_pressed();
}

void lcdDisplayI2C::move_cursor(const int col, const int row) {
  _lcd_i2c.setCursor(col, row);
}

// String lcdDisplayI2C::time_to_string(int lineNum) {
//   return time_to_string(timeDigits[lineNum]._val, timeDigits[lineNum+1]._val, timeDigits[lineNum+2]._val);
// }

// String lcdDisplayI2C::time_to_string(const String hour, const String min, const String sec) {
//   return hour + ':' + min + ':' + sec;
// }

String lcdDisplayI2C::zero_pad(const int time_val) {
  if (time_val < 10)
    return "0" + String(time_val);
  return String(time_val);
}

// void lcdDisplayI2C::init_time_digits() {
//   for (int i = 0; i < 6; i++) {
//     timeDigits[i]._val = 0;
//     timeDigits[i]._name = _timeDigitNames[i];
//   }
// }

// String lcdDisplay::display_time_string(timeDigit* time) {
//   return sprintf("%4d:%2d:%2d", time[0]._val, time[1]._val, time[2]._val)
// }

// lcdDisplay::~lcdDisplay() {
//   delete[] displayDigits;
// }

// lcdDisplay::displayDigit::displayDigit(const int val, const String name) {
//   setVal(val);
//   _name = name;
// }

// void lcdDisplay::displayDigit::getVal() {
//   return _val;
// }

// void lcdDisplay::displayDigit::setVal(const int val) {
//   _val = val;
// }

// void lcdDisplay::displayDigit::name() {
//   return _name;
// }
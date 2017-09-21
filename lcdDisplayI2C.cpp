#include "lcdDisplayI2C.h"
#include <Time.h>
//#include <TimeLib.h>

static const char TICK = '-';
static const char EMP = ' ';
static const int OFFSET = 4;
static const int ASCII_ZERO = 48;
static const int ASCII_FIVE = ASCII_ZERO+5;
static const int ASCII_NINE = ASCII_ZERO+9;

String menuLines[6] = {"Set Big Int", "Nxt Strt Time", "Set Small Int", "# of Times", "Set Open Time", "DISABLE"};
// default Settigs
String defaultSettings[5] = {"00:05:00", "00:02:00", "00:01:00", "04", "00:00:10"};
const int CURSOR_ROWMIN = 0;
const int CURSOR_ROWMAX = 5;


lcdDisplayI2C::lcdDisplayI2C(int addr, int col, int row, int up, int down, int left, int right, int enter, int activationPin) 
  : _lcd_i2c(addr, col, row), _upDown(down, up, "down", "up", "upDown"), _leftRight(right, left, "right", "left", "leftRight"),
  _enter(enter, "enter") {

  _activationPin = activationPin;
  pinMode(activationPin, OUTPUT);

  // TEST period and next startTime
  for (int i = 0; i < (sizeof(_timeSettings)/sizeof(*_timeSettings)); i++) {
      _timeSettings[i] = defaultSettings[i];
  }
  _lcd_i2c.begin();
  show_main_page();
}

// can only be called when on time setting page, printing out the new time digits
void lcdDisplayI2C::update_time_setting_line() {
  _lcd_i2c.setCursor(OFFSET, 1);
  _lcd_i2c.print(_timeSettings[_cursorRowPosition]);
}

// can only be called when on main page
void lcdDisplayI2C::move_tick(int tickPos) {
  for (int i = 0; i < 2; i++) {
    _lcd_i2c.setCursor(1, i);
    if (i == tickPos)
      _lcd_i2c.write(TICK);
    else
      _lcd_i2c.write(EMP);
  }
}

void lcdDisplayI2C::show_main_page() {
  render_main_page(false);
  move_tick(0);
  update_time_settings();

  Serial.println("Showing main page");

  int newCursorRowPosition;

  // // limit the up down movement of the cursor
  _upDown.setVals(_cursorRowPosition, CURSOR_ROWMIN, CURSOR_ROWMAX);

  // int c;
  while (true) {
    //if enter is pressed store the time settings
    if (enter_pressed()) {
      if (_cursorRowPosition == CURSOR_ROWMAX) {
        // DISABLE!
        if (menuLines[5] == "DISABLE") {
            // show enable option
            menuLines[5] = "ENABLE";
            enabled = false;
        } else {
          menuLines[5] = "DISABLE";
          enabled = true;
        }
        render_main_page(true);
        move_tick(1);

        continue;
      }
      show_time_setting_page();
      return;
    }

    newCursorRowPosition = _upDown.button_pressed(false);

    // update cursor position
    if (newCursorRowPosition != _cursorRowPosition) {
      
      // update the menu page when scrolling up and down
      if (newCursorRowPosition > _cursorRowPosition) {
        // scrolling downwards
        _cursorRowPosition = newCursorRowPosition;
        if (newCursorRowPosition % 2 == 0) {
          render_main_page(false); 
        }
      } else if (newCursorRowPosition < _cursorRowPosition) {
        _cursorRowPosition = newCursorRowPosition;
        // scrolling upwards
        if (newCursorRowPosition % 2 == 1) {
          render_main_page(true);
        }
      }
      _cursorRowPosition = newCursorRowPosition;

      move_tick(_cursorRowPosition%2);
    }
    
    unsigned long currentMillis = millis();
    // waitTime is the period
    unsigned long waitTime = _timeSettingsInNum[0];
    
    // first time
    if (!inPeriod) {
      // next start time
      waitTime = _timeSettingsInNum[1];
    }
    
    // activate solenoid valve
    if ((unsigned long)(currentMillis - previousMillis) >= waitTime) {
      Serial.println("ACTIVATE");
      previousMillis = currentMillis;

      unsigned long innerCurrentMillis = millis();
      unsigned long innerPreviousMillis = 0;
      unsigned long period = _timeSettingsInNum[2];
      unsigned long numOfTimes = _timeSettingsInNum[3];
      unsigned long duration = _timeSettingsInNum[4];

      int counta = 0;
      // only turn on when enabled
      if (enabled) {
        while (counta < numOfTimes) {
          innerCurrentMillis = millis();
          if ((unsigned long) (innerCurrentMillis - innerPreviousMillis) >= period) {
              // activate valve for 5 seconds (TODO: make activation customizable)
            Serial.println("TURN ON");
            Serial.println(innerPreviousMillis);
            Serial.println(innerCurrentMillis);


            innerPreviousMillis = innerCurrentMillis;
            digitalWrite(_activationPin, HIGH);
            
            delay(duration);
            digitalWrite(_activationPin, LOW);
            counta++;
          }
        }
      }
      inPeriod = true;
    }
  }
}

void lcdDisplayI2C::show_time_setting_page() {
  render_time_setting_page();
  for (int i = 0; i < (sizeof(_timeSettings)/sizeof(*_timeSettings)); i++) {
      Serial.println(_timeSettings[i]);
  }
  // set the limits of left and right
  if (_cursorRowPosition == 3)
    _leftRight.setVals(0, 0, 1);
  else
    _leftRight.setVals(0, 0, 5);

  int newLeftRightPos;
  int newUpDownVal;
  int COLON_OFFSETS = _cursorColPosition/2;
  // // set timeString to change
  char* timeString;
  timeString = &_timeSettings[_cursorRowPosition][0];
 
  int currentTimeDigit = (int) timeString[_cursorColPosition+COLON_OFFSETS];

  // set updown max value for the first digit upon entering mode
  _upDown.setVals(currentTimeDigit, ASCII_ZERO, ASCII_NINE);

  while (true) {
    newLeftRightPos = _leftRight.button_pressed(false);

    int oldVal = _upDown.getVal();
    newUpDownVal = _upDown.button_pressed(true);

    if (enter_pressed()) {

      inPeriod = false;
      // set previousMillis to the time exiting the time setting page
      previousMillis = millis();
      show_main_page();
    } else if (newLeftRightPos != _cursorColPosition) {
      _cursorColPosition = newLeftRightPos;

      COLON_OFFSETS = _cursorColPosition/2;

      // update cursor position
      move_cursor(_cursorColPosition+OFFSET+COLON_OFFSETS, 1);

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
      // Serial.print("currentTimeDigit: ");
      // Serial.println(currentTimeDigit);
      _upDown.setVals(currentTimeDigit, ASCII_ZERO, newUpDownMaxVal);
      // continue to next iteration
      continue;
    } else if (newUpDownVal != currentTimeDigit) {
        // constraints:
        // 1. big int >= # of Times * small int
        // 2. small int >= open time
        // 3. nxt strt time, # of times, open time all have minimum value of 1
      
      currentTimeDigit = newUpDownVal;
      // update time Strings
      timeString[_cursorColPosition+COLON_OFFSETS] = (char)newUpDownVal;

      // String s2;
      // int mult = 1;

      // switch(_cursorRowPosition) {
        
      //   case 0:
      //     s2 = _timeSettingsInNum[2];
      //     mult = _timeSettingsInNum[3];
      //     break;
      //   case 2:
      //     s2 = _timeSettingsInNum[4];
      //   case 1:
      //   case 3:
      //   case 4:
      //     // minimum value of 1 sec
      //     s2 = "00:00:01";
      //     break;
      //   default:
      //     break;
      // }

      // if (!lcdDisplayI2C::ifStringBeGreater(timeString, s2, mult))
      //   timeString[_cursorColPosition+COLON_OFFSETS] = (char) oldVal;
      
      update_time_setting_line();
      // reset cursor to the current digit
      move_cursor(_cursorColPosition+COLON_OFFSETS+OFFSET, 1);
      Serial.println(_cursorColPosition+COLON_OFFSETS+OFFSET);
    }
    delay(50);
  }   
}

// s1 >= s2*mult
bool lcdDisplayI2C::ifStringBeGreater(String s1, String s2, unsigned int mult) {
  return lcdDisplayI2C::get_millis(s1) >= lcdDisplayI2C::get_millis(s2)*mult;
}

void lcdDisplayI2C::update_time_settings() {
  for (int i = 0; i < (sizeof(_timeSettingsInNum)/sizeof(*_timeSettingsInNum)); i++) {
      if (i == 3) {
        _timeSettingsInNum[i] = _timeSettings[i].toInt();
        continue;
      }
      _timeSettingsInNum[i] = lcdDisplayI2C::get_millis(_timeSettings[i]);
  }
  // interval = get_millis(_bigInterval);
  // nextStartTimeFromNow = lcdDisplayI2C::get_millis(_nextStartTime);
}

unsigned long lcdDisplayI2C::get_millis(String timeString) {
  unsigned long hours = (unsigned long)(timeString.substring(0, 2).toInt())*36000000;
  unsigned long mins = (unsigned long)(timeString.substring(3, 5).toInt())*60000;
  unsigned long secs = (unsigned long)(timeString.substring(6, 8).toInt())*1000;
  return hours+mins+secs;
}

void lcdDisplayI2C::render_main_page(bool reverse) {
  // if reverse == false
  // render lines _cursorRowPosition and _cursorRowPosition+1
  // else
  // render lines _cursorRowPosition-1 and _cursorRowPosition
  _lcd_i2c.noCursor();
  _lcd_i2c.noBlink();
  _lcd_i2c.clear();
  
  move_cursor(3, 0);
  if (reverse)
    _lcd_i2c.print(menuLines[_cursorRowPosition-1]);
  else
    _lcd_i2c.print(menuLines[_cursorRowPosition]);

  move_cursor(3, 1);
  if (reverse)
    _lcd_i2c.print(menuLines[_cursorRowPosition]);
  else
    _lcd_i2c.print(menuLines[_cursorRowPosition+1]);
}

void lcdDisplayI2C::render_time_setting_page() {
  // starting from the beginning of time string
  _cursorColPosition = 0;
  _lcd_i2c.clear();

  // set the period that for the watering to be activated
  move_cursor(1, 0);
  _lcd_i2c.print(menuLines[_cursorRowPosition]);

  // set time from current time to start first cycle
  move_cursor(OFFSET, 1);
  _lcd_i2c.print(_timeSettings[_cursorRowPosition]);

  move_cursor(OFFSET, 1);
  _lcd_i2c.cursor();
  _lcd_i2c.blink();
}

bool lcdDisplayI2C::enter_pressed() {
  return _enter.button_pressed();
}

void lcdDisplayI2C::move_cursor(const int col, const int row) {
  _lcd_i2c.setCursor(col, row);
}


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "max6675.h"

// IO constants
const uint8_t THERMO_SO_PIN = 2;
const uint8_t THERMO_CS_PIN = 3;
const uint8_t THERMO_SCK_PIN = 4;
const byte SEG_14_ADDRESS = 0x70; // address for 14 seg display

// regular constants
const uint32_t REFRESH_RATE = 1000; // milliseconds

// variables - only for items that should exist outside of loop
uint32_t timerRefresh = 0; // for refreshing reading & display

// Device types (from libraries)
MAX6675 thermocouple(THERMO_SCK_PIN, THERMO_CS_PIN, THERMO_SO_PIN);
Adafruit_AlphaNum4 segDisplay = Adafruit_AlphaNum4();

// for 14 seg display
char displayBuffer[4] = "   "; // NULL is meant for pointers
// all other char[] need to have an additional index to mark the end (LF etc?). This does not need to be passed to the display buffer.

void setup()
{
    // setup display
    segDisplay.begin(SEG_14_ADDRESS);
    segDisplay.setBrightness(4);
    segDisplay.clear();
    segDisplay.writeDisplay();

    // display a turn on message & wait a second
    char startMessage[5] = "ON  "; // 5th element is LF
    for (uint8_t i = 0; i < 4; i++) {
        segDisplay.writeDigitAscii(i, startMessage[i]);
    }
    segDisplay.writeDisplay();
    delay(1000);
    segDisplay.clear();
}

void loop()
{
    uint8_t i;
    // read value based on a timer, avoid using delay()
    if (timerRefresh == 0) {
        timerRefresh = millis();
    }
    if ((timerRefresh > 0) && (millis() - timerRefresh) > REFRESH_RATE) {
        // read value from the thermocouple module & convert to int
        // there's a big assumption here that temperatures are only ever positive
        // but that's because it's for a heating element
        uint16_t temperature = round(thermocouple.readCelsius());

        // DISPLAY
        // move into temporary  character buffer
        sprintf(displayBuffer, "%d", temperature);

        // display buffer is left justified, but right justified looks better
        uint8_t digitsTemperature = 0;
        // bad method for figuring this out
        if (temperature < 10) {
            digitsTemperature = 1;
        } else if (temperature < 100) {
            digitsTemperature = 2;
        } else {
            digitsTemperature = 3; // probably
        }

        // write values justified correctly
        for (i = (4 - digitsTemperature); i < 4; i++) {
           segDisplay.writeDigitAscii(i, displayBuffer[i - (4 - digitsTemperature)]);
        }
        // null other values
        for (i = 0; i < (4 - digitsTemperature); i++) {
            char ch[2] = " ";
            segDisplay.writeDigitAscii(i, ch[0]);
        }
        segDisplay.writeDisplay();
        
        // reset
        timerRefresh = 0;
    }

}
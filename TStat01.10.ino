
/* 
** File: TStat01.10.ino
** Date: 10/06/2015
** By:   Mike Cipolla
**
** This is a HVAC Thermostat Application for the Arduino Mega with a LCD Touch Sheild
** This revision 1.10 adds Ethernet Remote Control
*/


#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008


#include "ioEther.h"
#include "ioEEProm.h"
#include "Relay.h"
#include "TStat.h"
#include "TouchButton.h"
#include "TxBox.h"
#include "TemperatureSensor.h"
#include "thTimer.h"
#include "TemperatureControl.h"
#include "Display.h"

#include "IoEEProm.h"

#define DEBUG

#ifdef DEBUG
char myMsg[80];
#endif

/*
************************************************************************************************************
*/
// Global Data
// Timers
// Caution:  Be sure to add any new clock timers to the initializeTimers() rotine so that clock rollover is prevented.
unsigned long previousClk_ms = 0L;
unsigned long previousClk125_ms = 0L;
unsigned long previousClk250_ms = 0L;
unsigned long previousOneSecClk_ms = 0L;
unsigned long previousTwoSecClk_ms = 0L;
unsigned long previousTempClk_ms = 0L;

// Define several timer durrations scaled to mili-seconds
const unsigned long time125_ms = 125L;
const unsigned long time250_ms = 250L;
const unsigned long timeOneSec = 1000L;
const unsigned long timeTwoSec = 2000L;
// 30 days is a time a little short of 50 days when the long timer will overflow
const unsigned long time30Days = 30 * 24 * 60 * 60 * 1000; 

boolean twoSecondStartupFlag;
TemperatureSensorClass myTSenor;
TemperatureControl tempControl;
ThTimer theTime;

// Buffers for EEProm Data
char CopyTcEEProm[255];  // Need size of at least TC_DATA_SIZE
char NewTcEEProm[255];

// External Delclarations
extern volatile unsigned long timer0_millis;   // Get direct access to the Arduino timer used by millis() so
//   a graceful reset can be performed before it rolls over.


/*
************************************************************************************************************
*/

void setup()
{
#ifdef DEBUG
	Serial.begin(9600);            //  This is for Debug
	Serial.print("Startup Now.");
	Serial.println();
#endif

	/*  
	** Configure the hardware
	*/
	if (REFERANCE_VOLTAGE == 1.1)
		analogReference(INTERNAL1V1);  // USE 1.1 Volt Reference since LM35 Output is < 1.0 V

	myTSenor.setInputPin(A8);	// Pin A8 Has the input from the LM35 Analog Temperature Sensor
								// Note:  This sensor input needs to have a pull down resistor 
								// (1k Ohm seems to work well)

	theTime.initializeTimers();   // Set up all the timers

	IoEEProm.readEE2Local((char*)&localEECopy);
	tempControl.restoreTcData((char*)&localEECopy);
	
	twoSecondStartupFlag = true;

	Display.init();
	Relay.init();

#ifdef DEBUG
	Serial.print("Startup is Done.");
	Serial.println();
#endif

}

/*
***************************************** MAIN LOOP ************************************************************
*/
void loop()
{
	char myRmTemp[40];
	unsigned long currentClk_ms = millis();


	/*                 // The 125ms Code Block //
	************************************************************************************************************
	*/
	if (currentClk_ms - previousClk_ms > time125_ms) {

		previousClk_ms = currentClk_ms;	

	}  // end of 125 ms block


	/*                 // The 250ms Code Block //
	************************************************************************************************************
	*/
	if (currentClk_ms - previousClk250_ms > time250_ms) {

		previousClk250_ms = currentClk_ms;
		
		myTSenor.readTempSensor();  // Read in, filter, and convert room temperature to degrees F

		Display.processButtons();
		Display.write();
	}


	/*                 // One Second Code Block  //
	************************************************************************************************************
	*/
	currentClk_ms = millis();

	if (currentClk_ms - previousOneSecClk_ms > timeOneSec) {
		previousOneSecClk_ms = currentClk_ms;

		if (millis() > time30Days)            // If we are getting close to timer overflow, then reset all timers
			theTime.initializeTimers();

	}  // end One Second Block


	/*                 // Two Second Code Block  //
	************************************************************************************************************
	*/
	currentClk_ms = millis();

	if (currentClk_ms - previousTwoSecClk_ms > timeTwoSec) {
		previousTwoSecClk_ms = currentClk_ms;
		twoSecondStartupFlag = false;
		  
		sprintf(myRmTemp, "Room Temp:%s", myTSenor.getTempTxt());
		TxBox(20, 10, 220, 22, YELLOW, BLUE, myRmTemp);
		u8 runCmd = tempControl.runControlRoomTemp();
		Relay.commandRelays(runCmd);

		// Keep EEProm Up To Date
		tempControl.copyTcData((char*)&localTcData);
		IoEEProm.readEE2Local((char*)&localEECopy);
		boolean EEPromUptodate = IoEEProm.areBuffsSame((char*)&localTcData, (char*)&localEECopy);
		if (!EEPromUptodate)
			IoEEProm.writeLocal2EE((char*)&localTcData);

#ifdef DEBUG
		Serial.print("tmpCmd = "); Serial.print(runCmd); Serial.print(myRmTemp); 
//		Serial.print(" EE="); Serial.print(EEPromUptodate);
		Serial.println();
#endif

	}  // end 2 second block

}  // End of MAIN LOOP
// EOF : Tstat01.00.ino
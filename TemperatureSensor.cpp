// File: TemperatureSensor.cpp
// By: Mike Cipolla
// Date: 10/09/2015
#ifndef __PRESURE
	#include "TemperatureSensor.h"
#endif


void TemperatureSensorClass::init()
{


}

void TemperatureSensorClass::setInputPin(uint8_t pin)
{
	inputPin = pin;
}

void TemperatureSensorClass::readTempSensor()
{
	//static float tempSmooth;
	int tempRead;
	float tempReadF;
	//Note: It is best to USE 1.1 Volt Reference if possible since LM35 Output is < 1.0 V	
	// Not set here // analogReference(INTERNAL1V1); // This would be the command for Arduino Mega
	tempRead = analogRead(inputPin);  // Read in voltage from pin A8

	tempReadF = ANALOG_LSB  * (float)tempRead;
	tempReadF /= TEMP_IN_CONVERT;
	rawTemp = tempReadF * 9.0 / 5.0 + 32;     // convert from deg. C to deg. F

	// filterTemperature();
	if (twoSecondStartupFlag == true)
		smoothTemp = rawTemp;  // Kick start the smooth temperature.
	smoothTemp = smoothTemp * 0.9375 + rawTemp * 0.0625;  // filter to smooth the temperature reading.
}

char* TemperatureSensorClass::getTempTxt()
{
	static char outMsg[40];

	int wholSmTemp = (int)(smoothTemp);
	int fracSmTemp = (int)((smoothTemp - wholSmTemp) * 10);
	sprintf(outMsg, "%d.%d F", wholSmTemp, fracSmTemp);
	return outMsg;
} 

float TemperatureSensorClass::getRoomTemp()
{
	return smoothTemp;
}

//  TemperatureSensorClass TemperatureSensor;


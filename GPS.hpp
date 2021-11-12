/*===================================================================================
Filename:	GPS.hpp
Version :	Draft A
Date	: 	11-November-2021
Author	:	Roger Thompson
====================================================================================*/

/*************************************************************************************
DESCRIPTION:
The connections between the Adafruit GPS Ultimate breakout board and the ESP32 are as
 follows:

Adafruit GPS   	Description				ESP32 		NOTES

	VPPS		One pulse/s output		-			Not used currently
	Vin			5V						-			Board has 3.3V regulator for chip
	Gnd			Ground					-
	RX			Data RX (UART#2) 			GPIO 17		RX forg configuration commands
	TX			Data TX (UART#2)  		GPIO 16		TX of  NMEA sentences to the ESP32
	FIX			FIX flag				-				Not used currently
	UBAT								-			Not used currently
	EN			Bit clock I/P			-			Not used currently
	3.3V out	3.3V output from onboard regulator	Not used currently

	Uses HardwareSerial "Serial2" on the ESP32 for I/O

*************************************************************************************/
#ifndef GPS_HPP
//------------------------------------------------------------------------------------

#include <Adafruit_GPS.h>

#include "AudioNav2.h"



Adafruit_GPS GPS(&Serial2); // connect GPS instance to ESP32 UART2

//int global;

// function to read and parse NMEA sentences from the GPS receiver
void getGPSdata(){
	// read from the UART one character at a time
	//global= 4;
	char c = GPS.read();
	if ((c) && (GPS_DEBUG)) Serial.write(c); // echo NMEA sentences to Serial monitor
	if (GPS.newNMEAreceived()) {
		if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
			return;  // we can fail to parse a sentence in which case we should just wait for another
		audioNav.GPSfix = GPS.fix;
		audioNav.GPSfixQuality = GPS.fixquality;
		audioNav.nSatellites = GPS.satellites;
		audioNav.speed 	= GPS.speed;
		audioNav.course = GPS.angle;
		if (GPS_DEBUG){
			Serial.print("$GPS  - ");
			Serial.print(audioNav.course); Serial.print("\t ");
			Serial.print(audioNav.speed); Serial.print("\t ");
			Serial.print(audioNav.GPSfix); Serial.print("\t ");
			Serial.print(audioNav.GPSfixQuality); Serial.print("\t ");
			Serial.print(audioNav.nSatellites); Serial.print("\n");
		}
	}
}

//-------------------------------------------------------------------------------------
#define GPS_HPP
#endif

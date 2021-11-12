/*===================================================================================
Filename:	Compass.hpp
Version :	Draft A
Date	: 	6-November-2021
Author	:	Roger Thompson
====================================================================================*/

/*************************************************************************************
DESCRIPTION:
This sketch defines the class 'Compass', which uses the HMC5883L magnetometer
to determine the compass heading of the principal horizontal axis of the AudioNav
enclosure.

The sketch reads data from the MHC5883L triple-axis magnetometer on the G-271
breakout board.  It is based on an example sketch written by Kevin Townsend for
Adafruit Industries

GY-271 pin connections:
    Vcc         5V rail (the GY-271 has an onboard 3.3V regulator for the MHC5883
    Ground      Ground rail
    SCL (I2C)   GPIO22 on the ESP32 (SCL on the Uno or A5 on the Nano)
    SDA (I2C)   GPIO21 on the ESP32 (SDA on the Uno  or A4 on the Nano)
    DRDY        N/C  (This is the Data Ready output, used to maximise sampling rate)

The I2C device address is 0x1E  (this value is set in the Adafruit_HMC5883_U.h file)

The nominal bandwidth for the HMC5883L is 15 samples per second.
*************************************************************************************/

#ifndef COMPASS_HPP
//------------------------------------------------------------------------------------


#include <Adafruit_HMC5883_U.h>
#include <Adafruit_Sensor.h>


#ifndef AUDIONAV2_H
	#include "AudioNav2.h"
	#define AUDIONAV2_H
#endif


#ifndef WIRE_H
	#include <Wire.h>
	#define WIRE_H
#endif

#ifndef ROLLING_AVERAGE_HPP
	#include "RollingAverage.hpp"
	#define ROLLING_AVERAGE_HPP
#endif


// The following calibration data is based on measurements made on 29/10/2021
#define X_OFFSET (+ 39.1)
#define Y_OFFSET (+ 32.75)
#define Y_SENSITIVITY_CORRECTION -0.9365

#define DECLINATION_ANGLE  0.0  // declination angle is essentially zero in Suffolk

// instantiate a magnetometer object
Adafruit_HMC5883_Unified magnetometer;


class Compass{
	// Member variables
	RollingAverage xRollingAv = RollingAverage(20);
	RollingAverage yRollingAv = RollingAverage(20);
	RollingAverage headingRollingAv = RollingAverage(20);
	float declinationAngle = 0.0;
	sensor_t sensor;
public:

	Compass(){  // explicit default constructor
		magnetometer.getSensor(&sensor);
	}

	bool begin(){
		// method to initialise the magnetometer
		bool status = false; // return status: OK = true;  Failed = false
		status = magnetometer.begin();
		if (status) {
			audioNav.compassActive = true;
		}else{
			if (MAG_DEBUG)
				Serial.println("$COMP: ERROR CODE 200 - Magnetometer not found");
		}
		return(status);
	}

	// getHeading method
	// Gets the raw values from the sensor; writes HEADING (deg) to audioNav struct
	void getHeading(){
		float xValue = 0, yValue = 0, zValue = 0;  		// raw values from the magnetometer
		float xRav = 0, yRav = 0, zRav = 0, hRav = 0; 	// rolling average values
		float heading = 0;
		float headingDegrees = 0;
		float declinationAngle = 0;

		//Get new sensor event
		sensors_event_t event;
		magnetometer.getEvent(&event);
		xValue = event.magnetic.x + X_OFFSET;
		yValue = (event.magnetic.y + Y_OFFSET) * Y_SENSITIVITY_CORRECTION;
		zValue = event.magnetic.z;
		xRav = xRollingAv.updateRav(xValue);
		yRav = yRollingAv.updateRav(yValue);

		heading = atan2(yValue, xValue);
		heading += DECLINATION_ANGLE;

		if(heading < 0) // Then correct for when signs are reversed.
			heading += 2*PI;
		// Check for wrap due to addition of declination.
		if(heading > 2*PI) // Correct for wrap due to addition of declination.
			heading -= 2*PI;
		headingDegrees = heading * 180/M_PI;
		hRav = headingRollingAv.updateRav(headingDegrees);

		if(MAG_DEBUG)
		{ //DIAGNOSTIC PRINTOUT:  Display  magnetic vectors in micro-Tesla (uT)
			Serial.print("$COMP:  X: ");
			Serial.print(xValue); Serial.print("\t "); Serial.print(xRav);
			Serial.print("\t  "); 	Serial.print("Y: ");
			Serial.print(yValue); Serial.print("\t "); Serial.print(yRav);
			Serial.print("\t  ");	Serial.print("Z: ");
			Serial.print(zValue); Serial.print(" uT");
			Serial.print("\tHeading (deg):  ");
			Serial.print(headingDegrees); Serial.print("\t "); Serial.println(hRav);
		}

		// finally write heading in (integer) degrees to the KEY VARIABLES struct
		audioNav.heading = headingDegrees;
		return;
	}

}; //end of class Compass


//------------------------------------------------------------------------------------
#define COMPASS_HPP
#endif

























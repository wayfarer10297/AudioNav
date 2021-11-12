/*====================================================================================
Filename:	AudioNav2.h
Version	:	Draft B
Date	: 	8-November-2021
Author	:	Roger Thompson
=====================================================================================*/


/*************************************************************************************
DESCRIPTION

Global pre-processor definitions and variable declarations for the AudioNav2 software

**************************************************************************************/
#ifndef AUDIONAV2_H
//-------------------------------------------------------------------------------------


#define GPS_DEBUG	0	// GPS diagnostic output ON/OFF
#define MAG_DEBUG	0	// Magnetometer diagnostic output ON/OFF
#define ANN_DEBUG	0	// Announcer diagnostic output ON/OFF
#define MEN_DEBUG	0	// Menu diagnostic output ON/OFF



// Data structure with the KEY VARIABLES involved in the operation of AudioNav
// These values are updated on a Scheduled basis by the associated functions/methods
struct AudioNav {
	// Compass
	bool compassActive = false;		// compass detected and initialised?
	float heading = 0;				// compass heading in degrees
	// GPS
	bool GPSfix = false;
	int nSatellites = 0;			// the number of satellites currently in use
	float GPSfixQuality = 100;		// the GPS horizontal Dilution of Precision
	float course = 0;				// Course Over the Ground (from GPS)
	float speed = 0.0;				// Speed Over the Ground in knots (from GPS)
	// Announcer
	int announcementInterval = 20;	// Periodic announcement interval in seconds
	// Menu
	int menuStatus = 0;				// 0 = menu system INACTIVE
	int mode = 2;					// mode 1 is simple announcement of COG & SOG
};

AudioNav audioNav;   // instantiate the KEY VARIABLES struct

//-------------------------------------------------------------------------------------
#define AUDIONAV2_H
#endif

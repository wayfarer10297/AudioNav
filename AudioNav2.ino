/*====================================================================================
Filename:	AudioNav2.ino
Version	:	Draft B
Date	: 	5-November-2021
Author	:	Roger Thompson
====================================================================================*/


/*************************************************************************************
DESCRIPTION

This is the master sketch for the prototype AudioNav (V0.2) unit.

It makes use of the Scheduler Class written by Bob Whitehouse to cue up (at the
appropriate intervals) the tasks needed to drive the hardware attached to the ESP32.

The code associated with the various ESP32 peripherals is, where possible,
encapsulated within Classes and/or Functions.  The code for these is contained within
separate dedicated .hpp files. These .hpp files are stored in the AudiNav2 Project
folder in the Sloeber(Eclipse) IDE.

The following 13 Arduino libraries are used by AudioNav:

	Adafruit_GPS_Library
	Adafruit_HMC5883_Unified
	Adafruit_Unified_Sensor
	ESP8266Audio
	FS
	HTTPClient  (needed for ESP8266Audio lib to compile(?), but not actively used)
	SD
	SPI
	SPIFFS
	WiFi
	WiFiClientSecure
	Wire
	LinkedList
*************************************************************************************/
#include "Arduino.h"

#include "AudioNav2.h"
#include "Scheduler.hpp"
#include "Announcer.h"
#include "Announcer.hpp"
#include "Compass.hpp"
#include "GPS.hpp"


Scheduler scheduler;			// Create an instance of the task Scheduler class
Compass compass; 				// Create an instance of the Compass class
Announcer announcer;			// Create an instance of the Announcer class


// Create the tasks to be scheduled
Task getGPSTask;				// Get GPS data (see GPS.hpp)
Task getCompassHeadingTask;  	// Get compass heading.  (see Compass.hpp)
Task getRotaryEncoderTask;  	// Rotary Encoder input and associated Menu actions
Task serveAnnouncerTask;  		// Service the Announcement queue
Task periodicAnnouncementTask;	// Scheduled periodic Navigation announcements
Task fixQualityAnnouncementTask;//Scheduled periodic update on GPS fix quality


//Task SDcardTask;
//Task MenuTask;


// task to update Compass heading
void getHead(){
	compass.getHeading();
}


// task to service the Announcement queue
void serveAnnouncer(){
	announcer.serviceQueue();
}


// task to cue up periodic navigation announcements
void periodicAnnouncement(){

	announcer.cuePeriodicAnnouncements();
}


void  GPSfixQualityAnnouncement(){
	announcer.announceGPSfixQuality();
}



//*******************************    setup()   *********************************************
void setup() {
	Serial.begin(115200);
	while(!Serial);
	WiFi.mode(WIFI_OFF);	// Disable Wi-Fi because it is incompatible with SPIFFS
	SPIFFS.begin();
	Serial.println("\nAudioNav starting...\n");

	// Initialise the Adafruit Ultimate GPS module using the Adafruit_GPS_Library
	Serial2.begin(9600,SERIAL_8N1,16,17);  // open the UART2 port on the ESP32 for GPS I/O
	while(!Serial2);

	GPS.begin(9600);
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // request only the RMC & GGA sentences
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // Set GPS update rate to 1Hz
	// GPS.sendCommand(PGCMD_ANTENNA);  // request updates on antenna status

	// Set up the getGPS Task to read the Compass to run once per second
    getGPSTask.proc = &getGPSdata;  // this function is in file GPS.hpp
	getGPSTask.repeating = true;
	getGPSTask.period = 1000;
	scheduler.add(&getGPSTask);

	//Initialise the Compass
	if (compass.begin()) {
		Serial.println("Compass active");
	}else{
		Serial.println("$MAIN: ERROR CODE 201 - Compass not found");
	}
	// Set up the getCompassHeading Task to read the Compass every 200ms
	getCompassHeadingTask.proc = &getHead;
	getCompassHeadingTask.repeating = true;
	getCompassHeadingTask.period = 2000000;
	scheduler.add(&getCompassHeadingTask);

	// Initialise the audio Announcer and make the first announcement
	load_speak_array();  // load pointers to the strings holding the .mp3 filenames
	//announcer.begin(AUDIONAV_STARTING);
	//if (mp3->isRunning()) Serial.println("mp3 is STILL RUNNING");
	audioLogger = &Serial;
	file = new AudioFileSourceSPIFFS(AUDIONAV_STARTING);
	id3 = new AudioFileSourceID3(file);
	out = new AudioOutputI2S(0,0,32,0);
	mp3 = new AudioGeneratorMP3();
	mp3 -> begin(id3, out);  //Announce that AudioNav is Starting

	// Set up the Task to service the Announcement queue every 50ms
	serveAnnouncerTask.proc = &serveAnnouncer;
	serveAnnouncerTask.repeating = true;
	serveAnnouncerTask.period = 50000;  // every 50ms
	scheduler.add(&serveAnnouncerTask);

	// cue up PERIODIC announcements every announcementInterval(seconds)
	periodicAnnouncementTask.proc = &periodicAnnouncement;
	periodicAnnouncementTask.repeating = true;
	periodicAnnouncementTask.period = audioNav.announcementInterval * 1000000;
	scheduler.add(&periodicAnnouncementTask);

	// cue up periodic announcement of GPSfixQuality
	fixQualityAnnouncementTask.proc = &GPSfixQualityAnnouncement;
	fixQualityAnnouncementTask.repeating = true;
	fixQualityAnnouncementTask.period = audioNav.announcementInterval * 1000000 * 5 ;
	scheduler.add(&fixQualityAnnouncementTask);


} // end setup()


void loop() {
	// Run the scheduler
	scheduler.dispatch();
} //end loop()

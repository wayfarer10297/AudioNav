/*====================================================================================
FilenameE:	Announcer.hpp
Version	:	Draft A
Author	:	Roger Thompson
Date	: 	30-October-2021
=====================================================================================*/


/**************************************************************************************
DESCRIPTION

This is the Audio Announcement module for the prototype AudioNav unit.

Audio output is achieved using the ESP8266Audio library routines.  The required speech
segments are stored as .mp3 files in SPIFFS.  The library routines retrieve the
required files from SPIFFS, convert them into PWM and then send this to the MAX98357A
class-D audio amplifier chip, using the I2S protocol.

The connections between the MAX98537A and the ESP32 are as follows:

MAX98357A   	Description				ESP32 		NOTES

  Vin			5V						n/a			range 2.5V - 5.5V (max)
  Gnd			Ground					n/a
  SD			Shut Down				GPIO14		Mute when grounded
  GAIN			Gain control			GPIO26		Adjust volume via DAC2 pin? (TBD)
  DIN			Data In	l				GPIO32		Digitised audio data Input
  BCLK			Bit clock				GPIO33		Bit clock Input
  LRC			L/R Clock				GPIO27		Frame select for L and R channels

**************************************************************************************/

#ifndef ANNOUNCER_HPP
//------------------------------------------------------------------------------------
#include <WiFi.h>
#include "SPIFFS.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include <cmath>
#include <string>
#include <sstream>

#include "Announcer.h"    // definitions of vocabulary and associated mp3 files

#define ANNOUNCEMENT_BUFFER_LENGTH	128

// Pointers to objects for the ESP8266 library
AudioFileSourceSPIFFS *file;	// ptr to AudioFileSourceSPIFFS instance
AudioGeneratorMP3 *mp3;			// ptr to AudioGeneratorMP3 instance
AudioFileSourceID3 *id3;		// ptr to AudioFileSourceID3 instance
AudioOutputI2S *out;			// ptr to AudioOutputI2S instance



class Announcer {
	// Cyclic queue to hold pending announcements
	const char* announcementQueue[ANNOUNCEMENT_BUFFER_LENGTH];
	int headQueueIndex = 0; 			// head of the announcement queue
	int tailQueueIndex = 0;				// tail of the announcement queue
	int announcementQueueLength = 0; 	// queue is empty

public:
	// Initialise the audio Announcer and play the initial announcement
	void begin(const char * mp3Filename){
		//audioLogger = &Serial;
		file = new AudioFileSourceSPIFFS(mp3Filename);
		id3  = new AudioFileSourceID3(file);
		out = new AudioOutputI2S(0,0,32,0);
		out -> SetPinout(33, 27, 32); 	// specify ESP32 pins used by the MAX98357A
		mp3 = new AudioGeneratorMP3();
		mp3 -> begin(id3, out);  // begin the announcement
		//if (mp3->isRunning()) Serial.println("mp3 is now RUNNING");
	}


	// method to add a new message to the queue
	void cueAnnouncement(const char* message)
	{
		announcementQueue[tailQueueIndex] = message;
		announcementQueueLength++;
		tailQueueIndex = ++tailQueueIndex%ANNOUNCEMENT_BUFFER_LENGTH;
		if(ANN_DEBUG)	Serial.println("$ANN - cueAnnouncement (msg added to queue)");
		if (announcementQueueLength > ANNOUNCEMENT_BUFFER_LENGTH)
		{
			Serial.println("ERROR CODE 300 - Announcement Queue overrun");
		}
	}




	// method to add PERIODIC navigation announcements to the queue (Mode dependent)
	// Modes are:  1:H only;  2: H+C+S;  3: C+S;  4: C only.
	void cuePeriodicAnnouncements()
	{
		if (audioNav.mode == 1 | audioNav.mode == 2)
		{
			cueAnnouncement(HEADING);
			announceNumber(audioNav.heading,3,0,true);
		}
		if ((audioNav.GPSfix) && (audioNav.mode == 2 | audioNav.mode == 4))
		{
			if (audioNav.speed >0.5){
				cueAnnouncement(COURSE);
				announceNumber(audioNav.course,3,0,true);
			}
		}
		if ((audioNav.GPSfix) && (audioNav.mode == 2 | audioNav.mode ==3))
			if(audioNav.speed > 0.5) {
				cueAnnouncement(SPEED);
				announceNumber(audioNav.speed,1,1,false);
				cueAnnouncement(KNOTS);
			} else {
				cueAnnouncement(CURRENTLY_STATIONARY);
			}
		if(ANN_DEBUG)	Serial.println("$ANN - End of cuePeriodicAnnouncements");
	} // End of cuePeriodicAnnouncements



	// method to SERVICE the announcement queue
	void serviceQueue()
	{
		if(ANN_DEBUG){
			Serial.print("$ANN - serviceQueue: Number of announcements waiting = ");
			Serial.println(announcementQueueLength);
		}

		if (mp3->isRunning())
		{
			if (!mp3->loop()) mp3->stop();
		}
		else
		{
			if(ANN_DEBUG) Serial.println("MP3 done ****");
			// if there is a message waiting AND the mp3 object from the previous  announcement has finsished transmisson
			if (announcementQueueLength > 0)
			{
				const char* mp3Filename;
				mp3Filename = announcementQueue[headQueueIndex];
				if(ANN_DEBUG){
					Serial.print("MakeAnnouncement:");
					Serial.println(mp3Filename);
				}
				// delete objects created for the previous announcement
				delete(file);
				//delete(id3);
				//delete(out);
				//delete(mp3);
				// create new objects for next announcement
				file = new AudioFileSourceSPIFFS(mp3Filename);
				//id3 = new AudioFileSourceID3(file);
				//out = new AudioOutputI2S(0,0,32,0);
				//mp3 = new AudioGeneratorMP3();
				// play announcement
				mp3 -> begin(id3, out);
				headQueueIndex = ++headQueueIndex%ANNOUNCEMENT_BUFFER_LENGTH;  // announcement buffer is cyclic; increment index modulo ANNOUNCEMENT_BUFFER_LENGTH
				announcementQueueLength--;
			}
		}
	}

	void announceNumber(float number, int intDigits, int fracDigits, bool leadingZeros){
		float intPart, fracPart;
		int numDigits, lZeros;
		if (fracDigits == 0) number += 0.5;  // for correct number rounding to integer
		fracPart = modf(number,&intPart);
		if (intPart == 0) numDigits = 0; else numDigits = int(log10(intPart)) + 1;


		// announce numbers of integer part
		for (int i = intDigits; i >= 1; i--){
			int digit;
			int divisor;
			divisor =  pow(10, i-1);
			digit = intPart/divisor;
			if ((i == 1)  &  (fracDigits > 0)){
				cueAnnouncement(speak[digit+50]);  		// add announcement of this digit+POINT to the queue
			}
			else
				cueAnnouncement(speak[digit]); 			// add announcement of this digit to the queue
			intPart = intPart - digit * divisor; 		//slice leading digit off of number
		}
		// announce integers of fractional part
		for (int i = 1; i <= fracDigits; i++){
			int digit;
			digit = int(fracPart * 10);
			cueAnnouncement(speak[digit]);
			fracPart = fracPart * 10  - digit;
		}
	}



	void announceGPSfixQuality(){
		if (audioNav.GPSfix == true ) {
			cueAnnouncement(SATELLITES_ACQUIRED);
			cueAnnouncement(speak[audioNav.nSatellites]);
			if (ANN_DEBUG) {
				Serial.print("Satellites:    "); Serial.println(audioNav.nSatellites);
			}
		}
		else	{
			cueAnnouncement(NO_GPS_FIX_AVAILABLE);
			return;
		}

		if (ANN_DEBUG) {
			Serial.print("DOP value:    "); Serial.println(audioNav.GPSfixQuality);}
		if (audioNav.GPSfixQuality <= 1) 	cueAnnouncement(FIX_QUALITY_EXCELLENT);
		else
			if (audioNav.GPSfixQuality <= 5) 	cueAnnouncement(FIX_QUALITY_GOOD);
			else
				if (audioNav.GPSfixQuality <= 10) 	cueAnnouncement(FIX_QUALITY_MODERATE);
				else
					if (audioNav.GPSfixQuality <= 20) 	cueAnnouncement(FIX_QUALITY_FAIR);
					else
						cueAnnouncement(FIX_QUALITY_POOR);
	}


}; // End of class Announcer


//-------------------------------------------------------------------------------------
#define ANNOUNCER_HPP
#endif


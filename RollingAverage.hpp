/*=====================================================================================
Filename:	RollingAverage.hpp
Version :	Draft A
Date	: 	2-November-2021
Author	:	Roger Thompson
=====================================================================================*/


/**************************************************************************************
DESCRIPTION:
A simple Class to calculate the rolling average of a series of float values.

The number of consecutive data points that are averaged is set at the point when the
RollingAverage object is instantiated, although this is currently subject to a
MAXIMUM of 50 because (for runtime efficiency) a static array is used to buffer the data
stream.

This function is used to smooth Magnetometer and GPS data where required.

 **************************************************************************************/

#define MAX_BUFFER_SIZE		50

class RollingAverage { // declaration/definition of the RollingAverage class
public:

	// member variables
	int rollingAverageLength;
	float buffer[MAX_BUFFER_SIZE];  // cyclic buffer to hold the last N readings
	int index; 	// the index for the cyclic buffer, labelling the next entry point
	float rollingTotal; // the sum of the last N readings



	// Default constructor
	RollingAverage() {
		rollingAverageLength = 1;
		buffer[MAX_BUFFER_SIZE] = {0};
		index = 0;
		rollingTotal = 0;
	}

	// Overloaded constructor used to specify the rollingAverageLength
	RollingAverage(int length) {
		/*if (length > MAX_BUFFER_SIZE)  {
			Serial.println("ERROR CODE 950: MAX_BUFFER_SIZE exceeded in RollingAverage");
			return;
		}*/
		rollingAverageLength = length;
		buffer[MAX_BUFFER_SIZE] = {0};
		index = 0;
		rollingTotal = 0;
	}

	//method to update rolling average with the latest reading
	float updateRav(float newValue) {
		rollingTotal -= buffer[index];  // subtract oldest value from rolling total
		rollingTotal += newValue;		// add the newValue to the rolling total
		buffer[index] = newValue;       // insert new value in the buffer
		index = (index + 1) % rollingAverageLength;
		return rollingTotal / rollingAverageLength;
	}
private:

};

/****************************************************************************
 * SecureSystem.c - this contains four functions for the touch sensor, led, and
 *              temperature sensor.
 *  Created on: Nov 16, 2019
 *      Author: Shao-Peng Yang
 ****************************************************************************/
#ifndef SECURESYSTEM_H_
#define SECURESYSTEM_H_

/*Public Resources*/
extern INT8U LEDCounter;
extern INT8U SensorState;
extern INT8U TScan;
extern INT8S TempC;
extern INT8S TempF;
extern INT8U AlarmLEDState;

/******************************************************************************
*SecureSystemInit - this initialize the ADC and Touch sensor
*Return Value: none
*Arguments: none
******************************************************************************/
void SecureSystemInit(void);
/******************************************************************************
*SenosrTask - This update the touch sensors state without blocking the system
*Return Value: none
*Arguments: none
******************************************************************************/
void SensorTask(void);
/******************************************************************************
*LEDTask - this handles the touch snesor LED patterns
*Return Value: none
*Arguments: none
******************************************************************************/
void LEDTask(void);
/******************************************************************************
*ADCTempTask - This read from the ADC and convert the value to degree celcius
*Return Value: none
*Arguments: none
******************************************************************************/
void ADCTempTask(void);
#endif /* SECURESYSTEM_H_ */

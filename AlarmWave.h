/**************************************************************************
 * AlarmWave.c - this contains two functions and a ISR for the alarm wave
 *              generation.
 *
 *  Created on: Nov 1, 2019
 *      Author: Shao-Peng Yang
 **************************************************************************/
#ifndef ALARMWAVE_H_
#define ALARMWAVE_H_

/******************************************************************************
*AlarmWaveInit - this initialize the DAC and PIT counter
*Return Value: none
*Arguments: none
******************************************************************************/
void AlarmWaveInit(void);
/******************************************************************************
*AlarmWaveControlTask - this task decide whether to turn on the PIT NVIC
*                       according to the alarm state so that the alarm will be
8                       generated in the PIT ISR
*Return Value: none
*Arguments: none
******************************************************************************/
void AlarmWaveControlTask(void);

#endif /* ALARMWAVE_H_ */

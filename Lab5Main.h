/*************************************************************************************************
* EE344 Lab 5
*   This lab create a security system with a key pad, touch sensor, temperature sensor,
*   and lcd interface. There are three states for the state machine: armed, disarmed and alarm.
*   The display will have check sum value printed at the begining, and the temperature value on
*   the LCD, user can switch from C to F with B button pressed. The system will have the touch
*   sensor enabled and the LED on them to indicate whether they are pressed. If a sensor is
*   touched or the temperature is outsied the range, the system will enter into alarm mode,
*   and an alarm signal will be played.
*
* Shao-Peng Yang, 11/23/2019
************************************************************************************************/

#ifndef LAB5MAIN_H_
#define LAB5MAIN_H_

/*Public Resource AlarmState*/
typedef enum {ARMED,DISARMED, ALARM} UISTATE_T;
extern UISTATE_T AlarmState;
/*1 for both LED off, 2 for one of them be on*/
extern INT8U LEDStartOver;

#endif /* LAB5MAIN_H_ */

# Secure-System-in-C
## Introduction
  In this lab, the designer will use the k65 tower board to create a secure system that integrated different sensor inputs and output devices such as LCD display and LEDs. The system includes reading data from the touch sensor and temperature sensor. It will send out certain output to the LCD display and the LEDs if the inputs reach a certain range or triggered by the user.  
  
  This system consists of real-time programming and the application of the co-operative kernel by using the function, SystickWaitEvent(), to create a task scheduler. Therefore, multitasking can be achieved and the CPU load can be reduced from free-running loops. When designing the program, the CPU load is the main concern for the system. Unnecessary action from the code will result in wasting CPU load which is avoided by the designer. 
## Program Description

SysTickWaitEvent():
	This function creates the scheduler for the system, which is divided into 10ms time slices. Everytime while entering the function, it will hold the CPU until the system timer reaches the beginning/end of the 10ms time slice. As a result, the period of the tasks will be an integer multiple of 10ms. The function is created by Professor Todd Morton. 
KeyTask():
  This task  is created by Professor Todd Morton. It scans the keypad and update
the keypad buffer. 

#### Lab5Main.c:
ControlDisplayTask():
  This task controls all of the prompts sent to the LCD display, and it also includes the state machine for the whole user interface. There are three states for the UI: ARMED, ALARM, and DISARMED. The corresponding prompts will be printed in different states on the first row of the LCD display. Three inputs are used in the task to decide the proper display message and the states for the system. The input from the touch sensor will change the state from ARMED to ALARM state. If the ‘D’ is pressed, the state of the system will change to DISARMED. When the ‘A’ is pressed and the current state is at DISARMED, the state changes to ARMED. In addition, the state will also change from ARMED to ALARM when the temperature is less than 0 or greater than 40 degrees Celsius. The LCD display will update the temperature every half second, and the display will switch between Celsius and Fahrenheit when ‘B’ is pressed.

CheckSum():
  This the memory contents from 0x00000000 to 0x0001ffff by summing up the data inside the memory. 

#### SecureSystem.c:

SensorTask():
	This task focuses on reading the touch sensor input data. There are two electrodes to be scanned. Since using free running loops in a scheduler isn’t a good practice, an non-blocking code is needed. Therefore, the task is switching from scanning electrode 1 and electrode 2. A software buffer is created to store the sensor data, and a flag to indicate the scanning is done is also included in this task. Later, those two public resources will be used together to determine whether the data in the buffer is valid and whether to change states. 
  
LED Task():
	The task controls the functionality of the LEDs. According to the states, the LEDs will have different blinking sequences. In ARMED, the LED will switch blinking every 250ms. In ALARM, the LED for the electrode triggered will be blinking every 100ms. Besides, any sensor that is triggered in ALARM will also enable the corresponding LEDs to blink in every 100ms. In DISARMED, if any sensors is touched, the corresponding LED blinks every 500ms. 

ADCTempTask():
  This task reads the value from the ADC and converts the data into temperature value by using Vin = 19.5*T - 400, where Vin = ADCval * 3.3 *10000 / (2^16). The ADC value will be limited when converting to the temperature value so that the range will be kept at -10 to 125 degrees celsius. If the value is out of range, the data will saturate to -10 or 125. 
  
  WatchDog: 
  Watchdog timer is initialized right before entering in the scheduler, so that it will be able to test the timing of the scheduler. At the end of the scheduler, there is a refresh task to refresh the watchdog timer in order to prevent the watchdog from resetting the system if the timing is passed. If the scheduler can’t reach the refresh task before the watchdog timer reaches the threshold value, the system will be reset and the flag in RCM will be set, which will be used to indicate whether the system is coming out of watchdog reset.

#### AlarmWave.c:
AlarmWaveControlTask():
	The task will enable NVIC for the ISR of a PIT timer in order to send out the alarm signal. The update on NVIC happens only when there is a state change, so that the NVIC won’t be enabled multiple times when the system is still in the ALARM state, or disabled multiple times in the other two states. 

PITHandler:
	The handler will be triggered every 52us by the PIT timer in order to send out a sample value from a lookup table to DAC to produce the alarm wave signal.
 
AlarmWave.c:
	DMA:
DMA is set up to replace the PIT handler so that the CPU load can be truly reduce to zero. With a proper configuration, DMA will send the sample value from the look up table to the DAC. It is configured to be triggered by the PIT counter every 52us so that the frequency of the alarm wave will be 300Hz.  

## Comments and Conclusions

Since there are multiple tasks in the system, the 10ms time slice might not be enough to include all of them in a time slice. Therefore, mutual exclusion is needed. The tasks might have the same execution period, but they can be distributed into different time slice so that it is possible to run all the tasks in the scheduler. In this system, ADCTempTask, AlarmWaveControlTask, and LEDTask and ControlDisplayTask are running in different time slices. Note that LEDTask needs to be triggered immediately after ControlDisplayTask so that it won’t miss any sensor value that changes the UI states. 
Also, since the system is running on a scheduler, it isn’t safe to have any free-running loop to occupy the CPU. As a result, when reading values from the ADC and touch sensor, two software buffers and flags are created to indicate whether the value inside the buffer is valid, so they can be properly used in the other tasks. This method reduces a huge amount of the CPU load since the conversion or scanning is done in the hardware, and the system isn’t waiting for the peripheral anymore. 

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
#include "MCUType.h"                                                                         //Include project header file
#include "Lab5Main.h"
#include "K65TWR_ClkCfg.h"
#include "Key.h"
#include "LCD.h"
#include "AlarmWave.h"
#include "K65TWR_GPIO.h"
#include "SysTickDelay.h"
#include "SecureSystem.h"

#define COL 1U
#define ROW2 2U
#define SEC 1000U

//typedef enum {AON,AOFF} UISTATE_T;                                                    //create states for the state machine
UISTATE_T AlarmState = ARMED;                                                            //define and initialize the current state for the state machin for the public reource

INT8U LEDStartOver = 2;

static INT16U CalcChkSum(INT8U *startaddr,  INT8U *endaddr);
static void ControlDisplayTask(void);


static const INT8C lab5CS[] = "CS: 0x";
static const INT8C lab5Armed[] = "ARMED";
static const INT8C lab5Disarmed[] = "DISARMED";
static const INT8C lab5Alarm[] = "ALARM";
static const INT8C lab5TempAlarm[] = "TEMP ALARM";
static const INT8C lab5NSign[] = "-";
static const INT8C lab5C[] = "\337C";
static const INT8C lab5F[] = "\337F";

void main(void){
    /*Check sum pointers and variable*/
    INT16U check_sum = 0;
    K65TWR_BootClock();

    /*Initialization for taskes*/
    KeyInit();
    LcdInit();
    AlarmWaveInit();
    GpioDBugBitsInit();
    GpioLED8Init();
    GpioLED9Init();
    SecureSystemInit();
    /*Checking Memory*/
    check_sum = CalcChkSum((INT8U *)0x00000000, (INT8U *)0x001fffff);
    LcdClrDisp();
    LcdMoveCursor(ROW2,COL);
    LcdDispStrg(lab5CS);
    LcdDispByte((INT8U)(check_sum >> 8));
    LcdDispByte((INT8U)check_sum);
    (void)SysTickDlyInit();
    SysTickDelay(SEC);
    LcdClrDisp();
    LcdDispStrg(lab5Armed);
    /*create time scheduler for the kernel*/
    while(1){
        SysTickWaitEvent(10);
        KeyTask();
        SensorTask();
        ControlDisplayTask();
        LEDTask();
        AlarmWaveControlTask();
        ADCTempTask();
    }
}
/***********************************************************************
*CalcChkSum() - Sums up the bytes stored in the memory from
*               the start addr to the end addr, this a private function
*Return Value: the 2bytes unsigned result of the summation
*Arguments: *startaddr is a pointer pointing to the data in the
*            start of the memory specified by users
*            *endaddr is a pointer pointing to the data in the
*            end of the memory specified by users
***********************************************************************/
static INT16U CalcChkSum(INT8U *startaddr, INT8U *endaddr){
    INT16U sum = 0;
    while(startaddr < endaddr)
    {
        sum += (INT16U)*startaddr;
        startaddr++;
    }
    sum += (INT16U)*startaddr;
    return sum;
}

/******************************************************************************
*ControlDisplayTask - this is the task that check the key every 50ms, and
*                   it changes the display message and states only when
*                   there is a change in keys we got('A' TO 'D' or 'D' TO 'A')
*                   this task also display the temperature data from the sensor,
*                   when 'B' is pressed, the display change mode from C to F or
*                   F to C
*Return Value: none
*Arguments: none
******************************************************************************/
static void ControlDisplayTask(void){

    /*time slice variable to make the period of the task be 50ms*/
    INT32U curr_slice = 0;
    static INT32U last_slice = 0;

    static INT8U dis_mode = 0;                                                                      //variable tracking the display mode for temperature
    INT8C cur_key = 0;                                                                              //holds value of the key pressed
    static INT8U last_mode = 1;                                                                     //used to check whether there is a change in displaying mode for temperature
    static INT8S last_temp = 0;                                                                     //used to check whether there is a change in temperature
    INT32U temp_disp = 0;                                                                           //used for holdingthe temperture value for displaying
    static INT8U temp_disp_count = 0;                                                               //used to create 500ms update for temp

    DB2_TURN_ON();
    curr_slice = SysTickGetSliceCount();                                                            //get the slice # of the current time slice
    if(curr_slice - last_slice >= 5){                                                               //check for 5 slices difference
        last_slice = curr_slice;
        cur_key = KeyGet();
        /*changing display mode for temp if 'B' is pressed*/
        if(cur_key == DC2){
            dis_mode ^= 1;
        }
        else{
        }

        /*displaying temperature value only when display mode changes or the temperature changes*/
        temp_disp_count++;
        if(((TempC != last_temp) || dis_mode!=last_mode) && temp_disp_count>=10){
            temp_disp_count = 0;
            if(dis_mode == 0){
                LcdClrLine(ROW2);
                if(TempC < 0 ){
                    LcdDispStrg(lab5NSign);
                    temp_disp = (INT32U)(~TempC + 1);                                               //2s complement on the temperature value for displaying negative value
                }
                else{
                    temp_disp = (INT32U)TempC;
                }
                LcdDispDecWord(temp_disp,0);
                LcdDispStrg(lab5C);
            }
            else if(dis_mode == 1){
                LcdClrLine(ROW2);
                temp_disp = (INT32U)(((INT16S)TempC*9)/5+32);                                       //changing Celcius to Fahrenheit
                LcdDispDecWord(temp_disp,0);
                LcdDispStrg(lab5F);
            }
            else{
            }
            last_temp = TempC;
            last_mode = dis_mode;
        }
        else{
        }

        /*state machine for the system*/
        switch(AlarmState){
            case(ARMED):
                /*Temperature triggering has higher priority than the touch sensor*/
                if(TempC > 40 || TempC < 0){
                    LcdClrLine(COL);
                    LcdDispStrg(lab5TempAlarm);
                    AlarmState = ALARM;
                    LEDStartOver = 1;                                                   //this initializes the LED state in SecureSystem module, 1 for both off, 2 for one of them to be on
                }
                else if(SensorState != 0 && TScan == 3){
                    LcdClrLine(COL);
                    LcdDispStrg(lab5Alarm);
                    AlarmState = ALARM;
                    LEDStartOver = 1;
                    LEDCounter = 0;                                                                     //used for the LED blinking timing in the SecureSystem module
                }
                else if(cur_key == DC4){
                    LcdClrLine(COL);
                    LcdDispStrg(lab5Disarmed);
                    AlarmState = DISARMED;
                    LEDStartOver = 1;
                    LEDCounter = 0;
                }
                else{
                }
                break;
            case(DISARMED):
                if(TempC > 40 || TempC < 0){
                    LcdClrLine(COL);
                    LcdDispStrg(lab5TempAlarm);
                    AlarmState = ALARM;
                    LEDStartOver = 1;
                }
                else if(cur_key == DC1){
                    LcdClrLine(COL);
                    LcdDispStrg(lab5Armed);
                    AlarmState = ARMED;
                    LEDStartOver = 2;
                    LEDCounter = 0;

                }
                else{
                }
                break;
            case(ALARM):
                if(cur_key == DC4){
                    LcdClrLine(COL);
                    LcdDispStrg(lab5Disarmed);
                    AlarmState = DISARMED;
                    AlarmLEDState = 0;
                    LEDStartOver = 1;
                    LEDCounter = 0;
                }
                else{
                }
                break;
            default:
                AlarmState = ARMED;
                LEDStartOver = 2;
                LEDCounter = 0;
                break;
        }                                                                          //used in SecureSsytem module to indicate the conversion of ADC is done
    }
    else{
    }
    DB2_TURN_OFF();
}


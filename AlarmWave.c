/**************************************************************************
 * AlarmWave.c - this contains two functions and a ISR for the alarm wave
 *              generation.
 *
 *  Created on: Nov 1, 2019
 *      Author: Shao-Peng Yang
 **************************************************************************/


#include "MCUType.h"
#include "Lab5Main.h"
#include "K65TWR_GPIO.h"
#include "AlarmWave.h"
#include "SysTickDelay.h"
void PIT0_IRQHandler(void);

/*The lookup table for the data of the 64 samples of the sine wave*/
static const INT16U awLookUpDac[64] = {
        32768,44544,54656,61751,65024,64359,60313,53981,46753,40023,34919,32100,31667,33196,35885,38769,40960,41843,41200,39230,36466,
        33614,31367,30223,30369,31642,33591,35599,37055,37511,36801,35078,32768,30458,28735,28025,28481,29937,31945,33894,35167,35313,
        34169,31922,29070,26306,24336,23693,24576,26767,29651,32340,33869,33436,30617,25513,18783,11555,5223,1177,512,3785,10880,20992
};
/******************************************************************************
*AlarmWaveInit - this initialize the DAC and PIT counter
*Return Value: none
*Arguments: none
******************************************************************************/
void AlarmWaveInit(void){

    SIM->SCGC6 |= SIM_SCGC6_PIT(1);
    SIM->SCGC2 |= SIM_SCGC2_DAC0(1);
    PIT->MCR = PIT_MCR_MDIS(0);
    PIT->CHANNEL[0].LDVAL = 3124;                                                           //60M/19200-1 to find the count
    PIT->CHANNEL[0].TCTRL = (PIT_TCTRL_TIE(1))|(PIT_TCTRL_TEN(1));

    DAC0->C0 |= (DAC_C0_DACEN(1)|DAC_C0_DACRFS(1)|DAC_C0_DACTRGSEL(1)|DAC_C0_DACSWTRG(1));
}

/******************************************************************************
*AlarmWaveControlTask - this task decide whether to turn on the PIT NVIC
*                       according to the alarm state so that the alarm will be
8                       generated in the PIT ISR
*Return Value: none
*Arguments: none
******************************************************************************/
void AlarmWaveControlTask(void){
    static INT8U counter = 1;                               //initialized to not happen at the same time with ther tasks
    static UISTATE_T LastAlarmState = ARMED;
    DB5_TURN_ON();
    counter++;
    if(counter >= 10){                                       //update every ten time slice, 100ms
        counter = 0;
        if(AlarmState != LastAlarmState){
            switch(AlarmState){
                case ALARM:
                    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF(1);
                    NVIC_ClearPendingIRQ(PIT0_IRQn);
                    NVIC_EnableIRQ(PIT0_IRQn);
                    break;
                case ARMED:
                    NVIC_DisableIRQ(PIT0_IRQn);
                    DAC0->DAT[0].DATL = 0x00;                                   //make sure the alarm is off
                    DAC0->DAT[0].DATH = 0x00;
                    break;
                case DISARMED:
                    NVIC_DisableIRQ(PIT0_IRQn);
                    DAC0->DAT[0].DATL = 0x00;                                   //make sure the alarm is off
                    DAC0->DAT[0].DATH = 0x00;
                    break;
                default:
                    break;
            }
            LastAlarmState = AlarmState;
        }
        else{
        }
    }
    else{
    }
    DB5_TURN_OFF();
}



void PIT0_IRQHandler(void){
    static INT8U table_count =0;                                        //track the index for the table
    static INT32U counter = 0;                                          // track for one second
    static INT8U on_off = 0x01U;                                        //bit to decide turn on or turn off the wave to generate "beep" for the alarm
    DB6_TURN_ON();
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF(1);
    if(counter < 19200 && on_off ==1){                                  //write samples to the DAC for a second
        counter++;
        DAC0->DAT[0].DATL = (INT8U)((awLookUpDac[table_count]>>4) & 0xFF);
        DAC0->DAT[0].DATH = (INT8U)(awLookUpDac[table_count]>>12);
    }
    else if (counter >= 19200){                                         //roll over once reach 1 second
        counter = 0;
        on_off ^= 0X01U;
    }
    else{
        counter++;
        DAC0->DAT[0].DATL = 0x00;                                       //turn on for half scale for the DAC
        DAC0->DAT[0].DATH = 0X08;
    }

    if(table_count < 63){
        table_count++;
    }
    else{
        table_count = 0;
    }

    DB6_TURN_OFF();
}



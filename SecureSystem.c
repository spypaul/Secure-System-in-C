/****************************************************************************
 * SecureSystem.c - this contains four functions for the touch sensor, led, and
 *              temperature sensor.
 *  Created on: Nov 16, 2019
 *      Author: Shao-Peng Yang
 ****************************************************************************/

#include"MCUType.h"
#include"SecureSystem.h"
#include"Lab5Main.h"
#include"K65TWR_GPIO.h"
#include "LCD.h"

#define RIGHTELEC 0U
#define LEFTELEC 1U
#define OFFSETONE 1280U
#define OFFSETTWO 1280U

INT8U LEDCounter = 0;                                                                       //Public resource for LED timing

INT8U SensorState = 0;                                                                      //This software buffer holds the state for the sensor
INT8U TScan = 0;                                                                            //this flag indicate whether the sensing is done for the touching sensor

INT8U AlarmLEDState = 0;                                                                    //this is used in the Alarm State for the LED display

INT8S TempC = 0;                                                                            //this holds the converted temperature value
INT8U ConvDone = 0;                                                                         //this indicates whether the conversion of ADC is done


/*value are assigned after the touch sensor is calibrated*/
static INT16U ssBaseLine[2] = {0,0};
static INT16U ssThreshold[2] = {0,0};

static const INT16S ssDivVal = 312;                                                         //for floating point arithmetic which is calculated by 19.5*2^4

/******************************************************************************
*SecureSystemInit - this initialize the ADC and Touch sensor
*Return Value: none
*Arguments: none
******************************************************************************/
void SecureSystemInit(void){

    /*ADC configuration*/
    SIM->SCGC6 |= SIM_SCGC6_ADC0(1);
    SIM->SOPT7 |= SIM_SOPT7_ADC0TRGSEL(5)|SIM_SOPT7_ADC0ALTTRGEN(1);                        //set up the ADC scanning clock to PIT

    /*PIT setup for the SDC*/
    PIT->CHANNEL[1].LDVAL = 29999999;
    PIT->CHANNEL[1].TCTRL = (PIT_TCTRL_TIE(1))|(PIT_TCTRL_TEN(1));

    /*set ADC to divided by 8, 16 bit conversion, long sample*/
    ADC0->CFG1 |= ADC_CFG1_ADLSMP(1) | ADC_CFG1_MODE(3) | ADC_CFG1_ADIV(3);
    /*hardware trigger*/
    ADC0->SC2 |= ADC_SC2_ADTRG(1);
    /*set up average mode*/
    ADC0->SC3 |= ADC_SC3_AVGE(1) | ADC_SC3_AVGS(3);
    /*start the first conversion*/
    ADC0->SC1[0] = ADC_SC1_ADCH(3);


    /*Touch Sensing Configuration*/
    SIM->SCGC5 |= SIM_SCGC5_TSI(1)|SIM_SCGC5_PORTB(1);
    PORTB->PCR[18] = PORT_PCR_MUX(0);
    PORTB->PCR[19] = PORT_PCR_MUX(0);

    TSI0->GENCS |= TSI_GENCS_REFCHRG(5)|TSI_GENCS_DVOLT(1)|TSI_GENCS_EXTCHRG(5)|TSI_GENCS_PS(5)|TSI_GENCS_NSCN(15);
    TSI0->GENCS |= TSI_GENCS_TSIEN(1);

    /*calibration for electrode 1*/
    TSI0->DATA  = TSI_DATA_TSICH(11); //electrode 2
    TSI0->DATA |= TSI_DATA_SWTS(1);
    while(!(TSI0->GENCS & TSI_GENCS_EOSF_MASK)){}
    TSI0->GENCS |= TSI_GENCS_EOSF(1);

    ssBaseLine[LEFTELEC] = (INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK);
    ssThreshold[LEFTELEC] = (INT16U)OFFSETTWO + ssBaseLine[LEFTELEC];

    /*calibration for electrode 2*/
    TSI0->DATA  = TSI_DATA_TSICH(12); //electrode 1
    TSI0->DATA |= TSI_DATA_SWTS(1);
    while(!(TSI0->GENCS & TSI_GENCS_EOSF_MASK)){}
    TSI0->GENCS |= TSI_GENCS_EOSF(1);

    ssBaseLine[RIGHTELEC] = (INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK);
    ssThreshold[RIGHTELEC] = (INT16U)OFFSETONE + ssBaseLine[LEFTELEC];

    /*start the first sensing process*/
    TSI0->DATA  = TSI_DATA_TSICH(12); //electrode 1
    TSI0->DATA |= TSI_DATA_SWTS(1);
    while(!(TSI0->GENCS & TSI_GENCS_EOSF_MASK)){}


}

/******************************************************************************
*SenosrTask - This update the touch sensors state without blocking the system
*Return Value: none
*Arguments: none
******************************************************************************/

void SensorTask(void){
    static INT8U slice_count = 0;
    static INT8U channel = 1;                                                                           //this indicate which channel is being scanned
    DB3_TURN_ON();
    slice_count++;
    if(slice_count >= 1){
        slice_count = 0;
        if(AlarmLEDState != 3){
            /*when the scan is done for each channel, it will update the buffer
             * and indicate the scan for the channel is done, clear the flag, start a new scan on the other channel*/
            if(TSI0->GENCS & TSI_GENCS_EOSF_MASK){
                if(channel == 2){
                    TSI0->GENCS |= TSI_GENCS_EOSF(1);
                    TScan |= 1<<LEFTELEC;
                    if((INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK) > ssThreshold[LEFTELEC]){
                        SensorState |= 1<<LEFTELEC;
                    }
                    else{
                        SensorState &= ~(INT8U)(1<<LEFTELEC);
                    }
                    TSI0->DATA  = TSI_DATA_TSICH(12); //electrode 1
                    TSI0->DATA |= TSI_DATA_SWTS(1);
                    channel = 1;
                }
                else if(channel == 1){
                    TSI0->GENCS |= TSI_GENCS_EOSF(1);
                    TScan |= 1<<RIGHTELEC;
                    if((INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK) > ssThreshold[RIGHTELEC]){
                        SensorState |= 1<<RIGHTELEC;
                    }
                    else{
                        SensorState &= ~(INT8U)(1<<RIGHTELEC);
                    }
                    TSI0->DATA  = TSI_DATA_TSICH(11); //electrode 2
                    TSI0->DATA |= TSI_DATA_SWTS(1);
                    channel = 2;
                }
                else{
                }
            }
        }
    }
    else{
    }
    DB3_TURN_OFF();
}

/******************************************************************************
*LEDTask - this handles the touch snesor LED patterns
*Return Value: none
*Arguments: none
******************************************************************************/
void LEDTask(void){
    static INT8U slice_count = 0;
    slice_count++;
    DB4_TURN_ON();
    if(slice_count >= 5){
        slice_count = 0;
        /*for the initial state o both LED, only happen once the state changes*/
        if(LEDStartOver == 1){
            LED8_TURN_OFF();
            LED9_TURN_OFF();
            LEDStartOver = 0;
        }
        else if(LEDStartOver == 2){
            LED8_TURN_ON();
            LED9_TURN_OFF();
            LEDStartOver = 0;
        }
        else{
        }
        switch(AlarmState){
            case(ARMED):
                LEDCounter++;
                /*two LED switch blinking for 250ms*/
                if(LEDCounter >= 5){
                    LEDCounter = 0;
                    LED8_TOGGLE();
                    LED9_TOGGLE();
                }
                else{
                }
                break;
            case(DISARMED):
                LEDCounter++;
                /*when the sensor is touched the LED blinks with 500ms*/
                if(LEDCounter >= 10 && TScan == 3){
                    LEDCounter = 0;
                    if(SensorState == 3){
                        LED8_TOGGLE();
                        LED9_TOGGLE();
                    }
                    else if(SensorState ==1){
                        LED8_TOGGLE();
                        LED9_TURN_OFF();
                    }
                    else if(SensorState == 2){
                        LED8_TURN_OFF();
                        LED9_TOGGLE();
                    }
                    else{
                        LED8_TURN_OFF();
                        LED9_TURN_OFF();
                    }
                    TScan = 0;
                }
                else{
                }
                break;
            case(ALARM):
                /*this upload the LED state for the alarm state, once LED is activated it will be on and blinking with 100ms*/
                if(AlarmLEDState != 3 && TScan == 3){
                    AlarmLEDState |= SensorState;
                    TScan = 0;
                }
                else{
                }
                LEDCounter++;
                if(LEDCounter >= 2){
                    LEDCounter = 0;
                    if(AlarmLEDState == 3){
                        LEDCounter = 0;
                        LED8_TOGGLE();
                        LED9_TOGGLE();
                    }
                    else if(AlarmLEDState == 1){
                        LEDCounter = 0;
                        LED8_TOGGLE();
                        LED9_TURN_OFF();
                    }
                    else if(AlarmLEDState == 2){
                        LEDCounter = 0;
                        LED8_TURN_OFF();
                        LED9_TOGGLE();
                    }
                    else{
                    }
                }
                else{
                }
                break;
            default:
                break;
        }
    }
    else{
    }
    DB4_TURN_OFF();
}

/******************************************************************************
*ADCTempTask - This read from the ADC and convert the value to degree celcius
*Return Value: none
*Arguments: none
******************************************************************************/
void ADCTempTask(void){
    static INT8U temp_count = 3;                                                    // initialized to not activate concurrently with other tasks
    INT16U sample = 0;
    INT16U vin = 0;
    DB7_TURN_ON();
    temp_count++;
    if(temp_count >= 50){                                                           //activated every 0.5s
        temp_count = 0;
        if(ADC0->SC1[0] & ADC_SC1_COCO_MASK){
            sample = ADC0->R[0];
            /*floating point calculation*/
            vin = (INT16U)(((INT32U)sample*3300) >> 16);
            TempC = (INT8S)(((INT16S)(vin - 400) << 4)/ssDivVal);
            /*limit the temp range from 125 to -10, measured and calculate value*/
            if(sample > 56351 ){
                TempC = 125; // calculated value is 56351
            }
            else if(sample < 4071){
                TempC = -10; // calculated value is 4071
            }
            ADC0->SC1[0] = ADC_SC1_ADCH(3);                                         //new conversion
        }
        else{

        }

    }
    else{
    }
    DB7_TURN_OFF();

}


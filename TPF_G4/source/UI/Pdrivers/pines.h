/***************************************************************************//**
  @file     pines.h
  @brief    Board management
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "UI/MCAL/gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** BOARD defines **********************************************************/

// On Board User LEDs
#define PIN_LED_RED     PORTNUM2PIN(PB, 22)    // PTB22
#define PIN_LED_GREEN   PORTNUM2PIN(PE, 26)    // PTE26
#define PIN_LED_BLUE    PORTNUM2PIN(PB, 21)    // PTB21

#define LED_ACTIVE      LOW
#define LED_DESACTIVE   HIGH



// On Board User Switches
#define PIN_SW2         PORTNUM2PIN(PC,6)     // PTC6
#define PIN_SW3         PORTNUM2PIN(PA,4)     // PTA4
#define MY_SW         	PORTNUM2PIN(PC,0)     // PTC0

#define SW_ACTIVE       HIGH



// DIGITALS INPUT/OUTPUT
//Freedom

/*                    LCD                     */
#define DIO_1         PORTNUM2PIN(PD, 1)	    // RS
#define DIO_2         PORTNUM2PIN(PD, 3)	    // EN
#define DIO_3         PORTNUM2PIN(PD, 2)	    // D4
#define DIO_4         PORTNUM2PIN(PD, 0)	    // D5
#define DIO_5         PORTNUM2PIN(PC, 4)	    // D6
#define DIO_6         PORTNUM2PIN(PA, 0)	    // D7

/*                  ENCODER                   */
#define DIO_7         PORTNUM2PIN(PC, 5)	    // A
#define DIO_8         PORTNUM2PIN(PC, 7)	    // B
#define DIO_9         PORTNUM2PIN(PC, 0)	    // D

/*                  BUTTONS                   */
#define DIO_10        PORTNUM2PIN(PB, 23)	    // PLAY
#define DIO_11        PORTNUM2PIN(PA, 1)	    // NEXT
#define DIO_12        PORTNUM2PIN(PB, 9)	    // PREV

/*                    UART                    */
#define DIO_13        PORTNUM2PIN(PC, 17)	    // Tx
#define DIO_14        PORTNUM2PIN(PC, 16)	    // Rx

/*                    DAC                     */
//#define DIO_15        PORTNUM2PIN(PB, 19)	    // PTB19

/*                    TEST PIN                */
#define DIO_16        PORTNUM2PIN(PB, 2)	    // IRQ

/*                    MATRIZ                  */
#define DIO_17        PORTNUM2PIN(PC, 2)	    // Din
#define DIO_18        PORTNUM2PIN(PC, 3)	    // Dout





/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_



/***************************************************************************//**
  @file     lcd.h
  @brief    LCD from SDK in NXP web
  @author   Micho
  @cite		https://community.nxp.com/t5/Kinetis-Software-Development-Kit/Driving-16x2-LCD-using-KSDK-drivers/ta-p/1098903
 ******************************************************************************/

#ifndef _LCD_
#define _LCD_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "MK64F12.h"
#include "hardware.h"

#include "UI/MCAL/gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

extern const unsigned char upper_line[];
extern const unsigned char lower_line[];
//volatile bool pitIsrFlag[2];


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void lcd_init(void);

void lcd_clear(void);

void lcd_write_upper_line(unsigned char* words);

void lcd_write_lower_line(unsigned char* words);

void lcd_Shift_Right(void);

void lcd_Shift_Left(void);


#endif // _LCD_

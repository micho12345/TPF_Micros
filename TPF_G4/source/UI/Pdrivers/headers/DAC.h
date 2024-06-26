/***************************************************************************//**
  @file     DAC.c
  @brief    DAC Driver for K64F.
  @author   Micho
 ******************************************************************************/

#ifndef _DAC_H_
#define _DAC_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "MK64F12.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef DAC_Type *DAC_t;
typedef uint16_t DACData_t;

typedef enum {
	DAC_0,
	DAC_1,
} DAC_n;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void initDAC(DAC_n channel);
void DACEnableDMA(DAC_n channel);
void DAC_SetData (DAC_n channel, DACData_t data);
void DAC_Enable(DAC_n channel, bool enable);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/



#endif // _DAC_H_

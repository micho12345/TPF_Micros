/***************************************************************************//**
  @file     DAC.c
  @brief    DAC Driver for K64F.
  @author   Micho
 ******************************************************************************/

#include <stdbool.h>
#include "MK64F12.h"
#include "hardware.h"
#include "UI/Pdrivers/headers/DAC.h"
#include "UI/MCAL/gpio.h"

/*******************************************************************************
 * 					CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DAC_DATL_DATA0_WIDTH 8


/***********************************************************************************************************
 * 									ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ***********************************************************************************************************/

/***********************************************************************************************************
 * 									VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ***********************************************************************************************************/



/***********************************************************************************************************
 * 													VARIABLES
 ***********************************************************************************************************/

static bool dac_init = false;


/***********************************************************************************************************
 * 									FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ***********************************************************************************************************/

void initDAC(DAC_n channel)
{
  if (dac_init){
        return;
    }
	dac_init = true;

  DAC_t dac = (channel == DAC_0) ? DAC0 : DAC1;

  if(channel == DAC_0)
		SIM->SCGC2 |= SIM_SCGC2_DAC0_MASK;
	else
		SIM->SCGC2 |= SIM_SCGC2_DAC1_MASK;

	dac->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;
}

void DACEnableDMA(DAC_n channel)
{
	DAC_t dac = (channel == DAC_0) ? DAC0 : DAC1;
	dac->C1 = DAC_C1_DMAEN_MASK;	// Enable DMA	// Enable DMA
}

void DAC_SetData (DAC_n channel, DACData_t data)
{
	DAC_t dac = (channel == DAC_0) ? DAC0 : DAC1;
	dac->DAT[0].DATL = DAC_DATL_DATA0(data);                          //ME MANDA DE A UN DATO
	dac->DAT[0].DATH = DAC_DATH_DATA1(data >> DAC_DATL_DATA0_WIDTH);
}


void DAC_Enable(DAC_n channel, bool enable)
{
	DAC_t dac = (channel == DAC_0) ? DAC0 : DAC1;
    if (enable) {
    	dac->C0 |= DAC_C0_DACEN_MASK;
    }
    else {
    	dac->C0 &= ~DAC_C0_DACEN_MASK;
    }
}


/************************************************************************************************************
* 										FUNCTION PROTOTYPES WITH LOCAL SCOPE
***********************************************************************************************************/


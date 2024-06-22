/***************************************************************************//**
  @file     PIT.c
  @author   Ignacio Cutignola
 ******************************************************************************/


#include "UI/Pdrivers/headers/PIT.h"
#include "hardware.h"
#include "MK64F12.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
static PIT_Type* PIT_ptr = PIT;
static SIM_Type* sim_ptr = SIM;

void (*callback0)(void);
void (*callback1)(void);
void (*callback2)(void);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void PIT_init(void)
{
	// Clock Gating
	sim_ptr->SCGC6 |= SIM_SCGC6_PIT_MASK;

    // 41.3.1 PIT Module Control Register
	PIT_ptr->MCR &= ~(PIT_MCR_MDIS_MASK | PIT_MCR_FRZ_MASK);
	PIT_ptr->MCR |= (PIT_MCR_MDIS(0) | PIT_MCR_FRZ(1));
}

void PIT_configTimer(uint8_t timer_id, uint16_t value)
{
    // Habilito la interrupcion
	NVIC_EnableIRQ(PIT0_IRQn);
	NVIC_EnableIRQ(PIT1_IRQn);
	NVIC_EnableIRQ(PIT2_IRQn);

    // 41.3.2 Timer Load Value Register
	PIT_ptr->CHANNEL[timer_id].LDVAL = (PIT_LDVAL_TSV(value - 1));

    // 41.3.4 Timer Control Register

    // 0 Timer is not chained.
	PIT_ptr->CHANNEL[timer_id].TCTRL &= ~(PIT_TCTRL_CHN_MASK);
	PIT_ptr->CHANNEL[timer_id].TCTRL |=(PIT_TCTRL_CHN(0));

    // 0 Interrupt requests from Timer n are disabled.
	PIT_ptr->CHANNEL[timer_id].TCTRL &= ~(PIT_TCTRL_TIE_MASK);
	PIT_ptr->CHANNEL[timer_id].TCTRL |=(PIT_TCTRL_TIE(0));

    // 0 Timer n is disabled.
	PIT_ptr->CHANNEL[timer_id].TCTRL &= ~(PIT_TCTRL_TEN_MASK);
	PIT_ptr->CHANNEL[timer_id].TCTRL |=(PIT_TCTRL_TEN(0));
}

void PIT_startTime(uint8_t timer_id)
{
    // 1 Interrupt will be requested whenever TIF is set.
	PIT_ptr->CHANNEL[timer_id].TCTRL &= ~(PIT_TCTRL_TIE_MASK);
	PIT_ptr->CHANNEL[timer_id].TCTRL |=(PIT_TCTRL_TIE(1));

    // 1 Timer n is enabled.
	PIT_ptr->CHANNEL[timer_id].TCTRL &= ~(PIT_TCTRL_TEN_MASK);
	PIT_ptr->CHANNEL[timer_id].TCTRL |=(PIT_TCTRL_TEN(1));
}

void PIT_stopTime(uint8_t timer_id)
{
    // 0 Interrupt requests from Timer n are disabled.
	PIT_ptr->CHANNEL[timer_id].TCTRL &= ~(PIT_TCTRL_TIE_MASK);
	PIT_ptr->CHANNEL[timer_id].TCTRL |=(PIT_TCTRL_TIE(0));

    // 0 Timer n is disabled.
	PIT_ptr->CHANNEL[timer_id].TCTRL &= ~(PIT_TCTRL_TEN_MASK);
	PIT_ptr->CHANNEL[timer_id].TCTRL |=(PIT_TCTRL_TEN(0));
}

void Pit_SetCallback(uint8_t timer_id, void(*funcallback)(void)) {
	switch (timer_id) {
		case 0:
			callback0 = funcallback;
			break;
		case 1:
			callback1 = funcallback;
			break;
		case 2:
			callback2 = funcallback;
			break;

	}
}


void PIT0_IRQHandler(void)
{
	callback0();
	PIT_ptr->CHANNEL[PIT_0].TFLG = PIT_TFLG_TIF(1);
}

void PIT1_IRQHandler(void)
{
	callback1();
	PIT_ptr->CHANNEL[PIT_1].TFLG = PIT_TFLG_TIF(1);
}

void PIT2_IRQHandler(void)
{
	callback2();
	PIT_ptr->CHANNEL[PIT_2].TFLG = PIT_TFLG_TIF(1);
}
/*******************************************************************************
 ******************************************************************************/

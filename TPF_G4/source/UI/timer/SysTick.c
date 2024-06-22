/***************************************************************************//**
  @file     SysTick.c
  @brief    Systyck configurator
  @author   Nicolas Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <UI/Pdrivers/pines.h>
#include "SysTick.h"
#include "../MCAL/gpio.h"
#include "MK64F12.h"

// Extern g_timeMilliseconds
#include "../../../sdmmc/port/event.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//#define SYSTICK_DEVELOPMENT_MODE    0

#define SYSTICK_LOAD_INIT ((__CORE_CLOCK__ / SYSTICK_ISR_FREQUENCY_HZ) - 1U)
#if SYSTICK_LOAD_INIT > (1 << 24)
#error Overflow de SysTick! Ajustar  __CORE_CLOCK__ y SYSTICK_ISR_FREQUENCY_HZ!
#endif // SYSTICK_LOAD_INIT > (1<<24)

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static systick_callback_t st_callback;

extern volatile uint32_t g_timeMilliseconds;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool SysTick_Init(systick_callback_t callback) {
	SysTick->CTRL = 0x00;			   //Enable sysT interrupt
	SysTick->LOAD = SYSTICK_LOAD_INIT; //1000L  - 1;
	SysTick->VAL = 0x00;
	NVIC_EnableIRQ(SysTick_IRQn);

	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

	st_callback = callback;

	return true;
}

__ISR__ SysTick_Handler(void) {
	g_timeMilliseconds++;
	#ifdef SYSTICK_DEVELOPMENT_MODE
		gpioWrite(PIN_IRQ, HIGH);
	#endif //SYSTICK_DEVELOPMENT_MODE
	if (st_callback!=NULL){
		st_callback();
	}
	#ifdef SYSTICK_DEVELOPMENT_MODE
		gpioWrite(PIN_IRQ, LOW);
	#endif //SYSTICK_DEVELOPMENT_MODE
}


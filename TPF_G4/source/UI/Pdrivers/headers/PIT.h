/***************************************************************************//**
  @file     PIT.h
  @author   Ignacio Cutignola
 ******************************************************************************/

#ifndef PIT_H_
#define PIT_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

typedef enum {
	PIT_0,
	PIT_1,
	PIT_2
} PIT_n;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief PIT_init: Inicializa el PIT
*/
void PIT_init(void);

/**
 * @brief PIT_configTimer: Configura el timer
 * @param timer_id
 * @param value: Se indica el tiempo dividido 20ns
 * @param callback
 */
void PIT_configTimer(uint8_t timer_id, uint16_t value);

void PIT_startTime(uint8_t timer_id);

void PIT_stopTime(uint8_t timer_id);

void Pit_SetCallback(uint8_t timer_id, void(*funcallback)(void));

#endif /* PIT_H_ */

/***************************************************************************//**
  @file     DMA.h
  @author   Ignacio Cutignola
 ******************************************************************************/

#ifndef DMA_H_
#define DMA_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {
	FTM0CH0 = 20, 	FTM0CH1 = 21, FTM0CH2 = 22, FTM0CH3 = 23,
	FTM0CH4 = 24, 	FTM0CH5 = 25, FTM0CH6 = 26, FTM0CH7 = 27,
	DMADAC0 = 45,	DMADAC1 = 46, DMAALWAYS63 = 63
} DMA_request_t;

typedef enum {
	DMA_0,
	DMA_1,
} DMA_n;

typedef struct DMA_config
{
  uint8_t  channel;
  uint16_t* source_buffer;
  uint16_t* destination_buffer;
  uint8_t source_offset;
  uint8_t transfer_size;
  uint32_t source_full_size;
  uint32_t source_unit_size;
  DMA_request_t request_source;
}DMA_config_t;


typedef void (*dma_callback_t)(void);

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void DMA_Config(DMA_config_t config);

void DMA_SetCallback(void(*funcallback)(void));

uint32_t DMA_GetRemainingMajorLoopCount(uint32_t channel);

#endif /* DMA_H_ */


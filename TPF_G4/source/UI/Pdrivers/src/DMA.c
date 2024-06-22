/***************************************************************************//**
  @file     DMA.h
  @author   Ignacio Cutignola ft Micho
 ******************************************************************************/

#include "UI/Pdrivers/headers/DMA.h"
#include "UI/MCAL/gpio.h"
#include "hardware.h"

void (*LoopCallback)(void);
void (*InputCapCallback)(void);
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void DMA_EnableRequest(uint8_t channel);
void DMA_DisableRequest(uint8_t channel);

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

static dma_callback_t DMA_callbacks [2];

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void DMA_Config(DMA_config_t config){
	// Habilito el clock gating
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

	// Habilito canal y setteo el request source con el FTM-CH0
	if(config.request_source >= 58) {
		DMAMUX->CHCFG[config.channel] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_TRIG_MASK | DMAMUX_CHCFG_SOURCE((uint8_t)config.request_source);
	}
	else {
		DMAMUX->CHCFG[config.channel] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE((uint8_t)config.request_source);
	}

	// Habilito las interrupciones
	//DMA0
	NVIC_ClearPendingIRQ(DMA0_IRQn);	// Elimino los eventos pendientes
	NVIC_EnableIRQ(DMA0_IRQn);			// Habilito propiamente la interrupcion
	//DMA1
	NVIC_ClearPendingIRQ(DMA1_IRQn);	// Elimino los eventos pendientes
	NVIC_EnableIRQ(DMA1_IRQn);			// Habilito propiamente la interrupcion

	// Configuro el origen
	DMA0->TCD[config.channel].SADDR = (uint32_t)(config.source_buffer); //List of Duties

	// Configuro el destino
	DMA0->TCD[config.channel].DADDR = (uint32_t)(config.destination_buffer);  // To change FTM Duty

	// Configuiro el offset para el origen y el destino
	DMA0->TCD[config.channel].SOFF = config.source_offset;
	DMA0->TCD[config.channel].DOFF =0x00;

	// Configuro la taza de trtansferencia del origen y destino
	switch(config.transfer_size) {
		case(1):
			DMA0->TCD[config.channel].ATTR = DMA_ATTR_SSIZE(0) | DMA_ATTR_DSIZE(0);
		break;
		case(2):
			DMA0->TCD[config.channel].ATTR = DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(1);
		break;
		case(4):
			DMA0->TCD[config.channel].ATTR = DMA_ATTR_SSIZE(2) | DMA_ATTR_DSIZE(2);
		break;
	}

	// Numero de bytes para ser transferidos en cada request
	DMA0->TCD[config.channel].NBYTES_MLNO = config.transfer_size;

	// Number of major transfer cycles
	DMA0->TCD[config.channel].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(config.source_full_size/config.source_unit_size);
	DMA0->TCD[config.channel].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(config.source_full_size/config.source_unit_size);


	if(config.channel == 0)
	{
		DMA0->TCD[config.channel].SLAST = -config.source_full_size;
	}
	if(config.channel == 1)
	{
		DMA0->TCD[config.channel].SLAST = 0;
	}


    // DLASTSGA DLAST Scatter y Gatter
    DMA0->TCD[config.channel].DLAST_SGA = 0x00;

	// Configuro registo de status y control
    DMA0->TCD[config.channel].CSR |= DMA_CSR_INTMAJOR_MASK;

	DMA0_EnableRequest(config.channel);

}

void DMA_SetCallback(dma_callback_t funcallback){
	DMA_callbacks[DMA_0] = funcallback;
}

//Sacado del INDU que lo que hace es tomar la cuenta de los bytes que se van transfiriendo
//https://github.com/vinodstanur/frdmk64f_mp3_player/blob/master/drivers/fsl_edma.c
uint32_t DMA_GetRemainingMajorLoopCount(uint32_t channel){
    uint32_t remainingCount = 0;

    if (DMA_CSR_DONE_MASK & DMA0->TCD[channel].CSR){
        remainingCount = 0;
    }
    else{
        /* Calculate the unfinished bytes */
        if (DMA0->TCD[channel].CITER_ELINKNO & DMA_CITER_ELINKNO_ELINK_MASK){
            remainingCount =
                (DMA0->TCD[channel].CITER_ELINKYES & DMA_CITER_ELINKYES_CITER_MASK) >> DMA_CITER_ELINKYES_CITER_SHIFT;
        }
        else{
            remainingCount =
                (DMA0->TCD[channel].CITER_ELINKNO & DMA_CITER_ELINKNO_CITER_MASK) >> DMA_CITER_ELINKNO_CITER_SHIFT;
        }
    }

    return remainingCount;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


void DMA_EnableRequest(uint8_t channel){
	if(channel == 0)
	{
		DMA0->ERQ &= ~(DMA_ERQ_ERQ0_MASK);
		DMA0->ERQ |= DMA_ERQ_ERQ0(1);
	}
	if(channel == 1)
	{
		DMA0->ERQ &= ~(DMA_ERQ_ERQ1_MASK);
		DMA0->ERQ |= DMA_ERQ_ERQ1(1);
	}

	DMAMUX->CHCFG[channel] &= ~(DMAMUX_CHCFG_ENBL_MASK);
	DMAMUX->CHCFG[channel] |= DMAMUX_CHCFG_ENBL(1);
}

void DMA_DisableRequest(uint8_t channel){
	if(channel == 0)
	{
		DMA0->ERQ &= ~(DMA_ERQ_ERQ0_MASK);
		DMA0->ERQ |= DMA_ERQ_ERQ0(0);
	}
	if(channel == 1)
	{
		DMA0->ERQ &= ~(DMA_ERQ_ERQ1_MASK);
		DMA0->ERQ |= DMA_ERQ_ERQ1(0);
	}

	DMAMUX->CHCFG[channel] &= ~(DMAMUX_CHCFG_ENBL_MASK);
	DMAMUX->CHCFG[channel] |= DMAMUX_CHCFG_ENBL(0);
}



void DMA0_IRQHandler(void){
	/* Clear the interrupt flag. */
	NVIC_ClearPendingIRQ(DMA0_IRQn);
	DMA0->CINT |= DMA_CINT_CINT(DMA_0);
	if(DMA_callbacks[DMA_0]){
		DMA_callbacks[DMA_0]();
	}
}

void DMA1_IRQHandler(void){
	/* Clear the interrupt flag. */
	NVIC_ClearPendingIRQ(DMA1_IRQn);
	DMA0->CINT |= DMA_CINT_CINT(DMA_1);
	if(DMA_callbacks[DMA_1]){
		DMA_callbacks[DMA_1]();
	}
}

void DMA_Error_IRQHandler(void){
	/* Clear all error indicators.*/
	DMA0->CERR = DMA_CERR_CAEI(1);
}

/*******************************************************************************
 ******************************************************************************/

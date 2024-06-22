/***************************************************************************//**
  @file     gpio.c
  @brief    GPIO Driver
  @author   Ignacio Cutignola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "MK64F12.h"
#include "hardware.h"
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// CLOCK ENABLE
#define PORT_ENABLE_MASK_CLK 0x200
#define PORT_LIMIT_MASK_CLK 0x3E00
#define CLK_CONTROL(port,x) (((uint32_t)(((uint32_t)(x)) << (9+port))) & PORT_LIMIT_MASK_CLK)


#define PORTX_IRQn(p) (PORTA_IRQn+p)
#define PINS_PER_PORT 32
#define ARRAY_SIZE (FSL_FEATURE_SOC_PORT_COUNT*PINS_PER_PORT)

static PORT_Type* const PORT_PTRS[FSL_FEATURE_SOC_PORT_COUNT] = PORT_BASE_PTRS;
static GPIO_Type* const GPIO_PTRS[FSL_FEATURE_SOC_PORT_COUNT] = GPIO_BASE_PTRS;

static pinIrqFun_t CALLBACKS[ARRAY_SIZE];

static void IRQHandler(int32_t port);

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void gpioMode (pin_t pin, uint8_t mode)
{

    // Verifico que se ingresaron los datos correctos
    if ( pin > PORTNUM2PIN(PE,31) ) return;

    // Obtengo el puerto y el numero de pin
    uint32_t port = PIN2PORT(pin);
    int num_pin = PIN2NUM(pin);

    //----------------------- Clock Enable ----------------------------
    //                      12.2.12 (pag. 323)
    SIM->SCGC5 |= CLK_CONTROL(port,1);
    //-----------------------------------------------------------------

    //-------------------- PORT configuration -------------------------
    //                      11.5.1 (pag. 288)

	PORT_Type* port_ptr = PORT_PTRS[port];

    // Vacio el puerto:
	port_ptr->PCR[num_pin] = 0x0;                   // Tener cuidado, se puso todo en 0!

    // MUX:
    port_ptr->PCR[num_pin] |= PORT_PCR_MUX(0b001);  // 001 Alternative 1 (GPIO).

    // DSE:
    port_ptr->PCR[num_pin] |= PORT_PCR_DSE(0b1);    // 1 High drive strength is configured on the corresponding pin, if pin is configured as a digital output.

    // SRE:
    // Ver final del codigo

    // PS & PE:
    if ((mode==INPUT_PULLDOWN)||(mode==INPUT_PULLUP))
    {
		port_ptr->PCR[num_pin] |= PORT_PCR_PE(0b1);     // 1 Internal pullup or pulldown resistor is enabled on the corresponding pin, if the pin is configured as a digital input.
        if(mode==INPUT_PULLDOWN)
        {
            port_ptr->PCR[num_pin] |= PORT_PCR_PS(0b0); // 0 Internal pulldown resistor is enabled on the corresponding pin, if the corresponding PE field is set.
        }
        else if(mode==INPUT_PULLUP)
        {
            port_ptr->PCR[num_pin] |= PORT_PCR_PS(0b1); // 1 Internal pullup resistor is enabled on the corresponding pin, if the corresponding PE field is set.
        }
	}
    //-----------------------------------------------------------------

    //----------------------- GPIO configuration ----------------------
    //                        55.2.6 (pag. 1803)
    GPIO_Type* gpio_ptr = GPIO_PTRS[port];
    if(mode == OUTPUT)
    {
        gpio_ptr->PDDR |= 1 << PIN2NUM(pin);
    }
    else if ((mode==INPUT_PULLDOWN)||(mode==INPUT_PULLUP))
    {
        gpio_ptr->PDDR |= 0 << PIN2NUM(pin);
    }
    //-----------------------------------------------------------------

    return;

}

void gpioWrite (pin_t pin, bool value) {
    if ( !(pin > PORTNUM2PIN(PE,31)) ){
		uint32_t new_value =  (uint32_t)(1 << PIN2NUM(pin));
		GPIO_Type* gpio_ptr = GPIO_PTRS[PIN2PORT(pin)];
		if(value) {
			gpio_ptr->PSOR = new_value;
		}
		else {
			gpio_ptr->PCOR = new_value;
		}
    }
}

bool gpioRead (pin_t pin) {
    if ( pin > PORTNUM2PIN(PE,31) ) return false;
    GPIO_Type* gpio_ptr = GPIO_PTRS[PIN2PORT(pin)];
    uint32_t pin_read = gpio_ptr->PDIR;     //Leo el puerto
    bool readed_pin = pin_read>>(PIN2NUM(pin)) & 0b1; //mando el pin deseado a la posiciÃ³n menos significativa, y la comparo con un 1
    return readed_pin;  //devuelvo el valor del pin resultante
}

void gpioToggle (pin_t pin) {   //no hay mucha magia, es lo mismo que el Write pero sin el if
    if ( !(pin > PORTNUM2PIN(PE,31)) ){
    	GPIO_Type* gpio_ptr = GPIO_PTRS[PIN2PORT(pin)];
    	gpio_ptr->PTOR = (uint32_t)(1 << PIN2NUM(pin));
    }
}


bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun) {

    // Verifico que se ingresaron los datos correctos
	if ( pin > PORTNUM2PIN(PE,31) ) return false;
	if ( irqMode >= GPIO_IRQ_MODE_HIGH ) return false;

    // Obtengo el puerto y el numero de pin
    uint32_t port = PIN2PORT(pin);
    const pin_t num_pin = PIN2NUM(pin);

    // Defino el puntero
	PORT_Type* port_ptr = PORT_PTRS[port];

    // Borro el flag
	uint32_t pcr_clear_irqc = (port_ptr->PCR[num_pin] & ~PORT_PCR_IRQC_MASK);
	port_ptr->PCR[num_pin] = pcr_clear_irqc | PORT_PCR_IRQC(irqMode) | PORT_PCR_ISF_MASK;

    // Habilito o deshabilito la interrupcion
	if (irqMode == GPIO_IRQ_MODE_DISABLE) {	NVIC_DisableIRQ(PORTX_IRQn(port)); }
	else { NVIC_EnableIRQ(PORTX_IRQn(port)); }

	CALLBACKS[PINS_PER_PORT*port + num_pin] = irqFun;

	return true;
}


static void IRQHandler(int32_t port) {

	PORT_Type* port_ptr = PORT_PTRS[port];
	uint32_t ISFR = port_ptr->ISFR;
	for(int pin = 0; pin<PINS_PER_PORT; pin++) {
		if (ISFR>>pin & 0b1) {
			port_ptr->ISFR |= 1<<pin; //w1c
			(*CALLBACKS[PINS_PER_PORT*port + pin])();
		}
	}
}

__ISR__ PORTA_IRQHandler(void) { IRQHandler(PA); }
__ISR__ PORTB_IRQHandler(void) { IRQHandler(PB); }
__ISR__ PORTC_IRQHandler(void) { IRQHandler(PC); }
__ISR__ PORTD_IRQHandler(void) { IRQHandler(PD); }
__ISR__ PORTE_IRQHandler(void) { IRQHandler(PE); }


//----------------------------------------- Comentarios extras -------------------------------------------

/*
    SRE:
    El SRE no se configura porque necesitamos que este en 0. Recordemos que segun Nico, el SRE es como un
    "freno de mano", si esta en Fast(0) reacciona rapido, si esta en Slow(1) tarda mucho en accionarse. Nostotros
    necesitaDAmos que sea rapido
*/

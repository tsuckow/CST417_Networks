#include <LPC23xx.H>
#include "isr.h"
#include "led/led.h"
#include "network/ethernet.h"

void irq_interrupt_service_routine(void)
{
	static int ledval = 0;
	if( T0IR & 0x01 ) //If Timer 0 Match
	{
		LED_Out(ledval++);	
		T0IR = 1;//Reset Interrupt
	}

	if( MAC_INTSTATUS )
	{
		ethernet_interrupt_handler();
	}
	
	VICVectAddr /*VICAddress*/ = 0; /*Ack the interrupt*/
    return;
}

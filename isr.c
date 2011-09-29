#include <LPC23xx.H>
#include "isr.h"
#include "network/ethernet.h"

__irq void irq_interrupt_service_routine(void)
{
	if( T0IR & 0x01 ) //If Timer 0 Match
	{	
		T0IR = 1;//Reset Interrupt
	}

	if( MAC_INTSTATUS )
	{
		ethernet_interrupt_handler();
	}
	
	VICVectAddr /*VICAddress*/ = 0; /*Ack the interrupt*/
    return;
}

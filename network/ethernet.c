#include "ethernet.h"
#include "EMAC.h"
#include "isr.h"
#include <LPC23xx.h>

#include <stdio.h>

void ethernet_init(void)
{
	Init_EMAC();

	//Interrupt
	VICVectAddr21  = (unsigned long)irq_interrupt_service_routine;
  	VICVectPriority21  = 15;
  	VICIntEnable  = (1  << 21);
}

void ethernet_interrupt_handler(void)
{
	printf("!! Ethernet Interrupt");
	MAC_INTCLEAR  = 0xFFFF;	
}

#include <LPC23xx.H>
#include "isr.h"
#include "led/led.h"
#include "network/ethernet.h"
#include <stdio.h>
#include <rtos.h>
#include <SROSpp/irq_handler.hpp>

irq_handler irqs;

extern "C" void irq_interrupt_service_routine(void)
{
	static int ledval = 0;
	if( T0IR & 0x01 ) //If Timer 0 Match
	{
		LED_Out(ledval++);	
		timerTick();//Tell SROS about the timer tick.

		T0IR = 1;//Reset Interrupt
		
	}

	irqs.process();
	
	VICVectAddr /*VICAddress*/ = 0; /*Ack the interrupt*/
    return;
}

#include <LPC23xx.H>                    /* LPC23xx definitions                */

#include <stdio.h>
#include <stdlib.h>

#include <rtos.h>
#include <rwLock.h>

#include <led/led.h>
#include <serial/serial.h>
#include <lcd/lcd.h>
#include "timer.h"

#include "SROSpp/threadfactory.hpp"
#include "SROSpp/mailbox.hpp"
#include "SROSpp/ethernet.hpp"

#include "isr.h"
#include "SROSpp/ethernet_driver_lpc23xx.hpp"
#include "SROSpp/ethernet_handler.hpp"
#include "SROSpp/arp.hpp"

//http://www.keil.com/support/man/docs/armlib/armlib_Chdfjddj.htm

ThreadFactory threadfactory;

ARP_Listener arp;

void ethernetSender();
void ethernetReceiver();
void arpSender();

Ethernet_Driver * const eth0 = new Ethernet_Driver_LPC23xx();
Ethernet_Handler eth_handler( eth0 );

extern "C" int _mutex_init_calls;
extern "C" int _mutex_aqui_calls;
extern "C" int _mutex_rele_calls;

int main(void)
{
	LED_Init();
	LED_Out(0x55);

	init_serial();

	printf("\n\n\n\n==MAIN==\n");

  lcd_init();
  lcd_clear();
  lcd_print (">>> NETWORKS <<<");
  set_cursor (0, 1);
  lcd_print (">>>>>>>><<<<<<<<");


   rtosInit();
   printf("SROS Init Complete\n");

   srand(1);
   timer_init();

   //System threads
   threadfactory.spawnThread(1000, 1,ethernetReceiver);
   threadfactory.spawnThread(100, 10,ethernetSender);
   threadfactory.spawnThread(1000, 20,arpSender);

   eth_handler.addListener( &arp );

   eth0->install( irq_interrupt_handler );
   irqs.add( eth0 );

   printf("MUTEX: %d %d %d\n", _mutex_init_calls, _mutex_aqui_calls, _mutex_rele_calls);

   printf("#Starting Scheduler\n");
   scheduler();            //This function will never return.
}

void ethernetReceiver()
{
	eth0->init();
	
	while(true)
	{
		eth0->recv_wait( -1 );//Wait forever
		
		while( eth0->isFrameAvailable() )
		{
			//This could be more OO so the end happens out of scope,
			//but it gets more interesting to enforce reading safely.
			unsigned short framesize = eth0->beginReadFrame();
			{
				uint8_t * framedata = eth0->readFramePointer();
				
				EthernetFrame frame( framedata, framesize );
				
				eth_handler.handleRecv( &frame );
				
				/*
				printf( "\n--- Ethernet Data ---\n" );
				for( int i = 0; i < framesize; ++i )
				{
					if( (i % 24) == 0 )
					{
						printf( "\n" );
					}
					
					printf( "%0.2x ", framedata[i] );
				}
				printf( "\n--- DONE ---\n" );
				*/
			
			}
			eth0->endReadFrame();
		}
	}
	
	
	
				
}

void ethernetSender()
{
	eth_handler.sender();
}

void arpSender()
{
	uint8_t frame[ 500 ];
	
	size_t i = 0;
	frame[i++] = 0xFF;
	frame[i++] = 0xFF;
	frame[i++] = 0xFF;
	frame[i++] = 0xFF;
	frame[i++] = 0xFF;
	frame[i++] = 0xFF;
	frame[i++] = 0x01;
	frame[i++] = 0x02;
	frame[i++] = 0x03;
	frame[i++] = 0x04;
	frame[i++] = 0x05;
	frame[i++] = 0x06;
	frame[i++] = 0x08;
	frame[i++] = 0x06;
	
	EthernetFrame eframe(frame, 64);
	
	while( true )
	{
		eth_handler.sendFrame( &eframe );
		sleep(1000);
	}
}

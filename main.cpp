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
#include "SROSpp/arp_handler.hpp"

ThreadFactory threadfactory;

void ethernetSender();
void ethernetReceiver();
void arpRequestThread();
void arpRecieveThread();
void userThread();

uint8_t const mymacaddr[6] = {0x00,0x11,0x22,0x33,0x44,0x55};
Ethernet_Driver * const eth0 = new Ethernet_Driver_LPC23xx(mymacaddr);
Ethernet_Handler eth_handler( eth0 );

uint8_t const myipaddr[4] = {192,168,0,13};
ARP_Handler arp_handler( &eth_handler, myipaddr );

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
   threadfactory.spawnThread(30, 1,ethernetReceiver);
   threadfactory.spawnThread(30, 10,ethernetSender);
   //threadfactory.spawnThread(1000, 20,arpSender);
   threadfactory.spawnThread(200, 21,arpRequestThread);
   threadfactory.spawnThread(200, 20,arpRecieveThread);
   threadfactory.spawnThread(200, 100,userThread);

   eth_handler.addListener( &arp_handler );

   eth0->install( irq_interrupt_handler );
   irqs.add( eth0 );

   printf("#Starting Scheduler\n");
   scheduler();            //This function will never return.
}

void ethernetReceiver()
{
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
			}
			eth0->endReadFrame();
		}
	}
}

void ethernetSender()
{
	eth0->init();
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

void arpRequestThread()
{
	arp_handler.requestThread();
}

void arpRecieveThread()
{
	arp_handler.packetRecieveThread();
}

void userThread()
{
	unsigned char addr[4] = {192,168,0,123};
	IPAddress iaddress( addr );
	EthernetAddress eaddress;

	while(true)
	{
		eaddress = arp_handler.request( iaddress );
		eaddress.print();
		printf("\n");
		sleep(3000);
	}
}


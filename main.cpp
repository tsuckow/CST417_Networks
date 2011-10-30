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

//http://www.keil.com/support/man/docs/armlib/armlib_Chdfjddj.htm

ThreadFactory threadfactory;
struct Test
{
	uint32_t a;
	uint32_t b;	
};

Mailbox<EthernetFrame> eth_incoming(4);
Mailbox<Test> box(2);

void threadA( Mailbox<Test> * box );
void threadB( Mailbox<Test> * box );

void ethernetSender();
void ethernetReceiver( Mailbox<EthernetFrame> * eth_incoming );

Ethernet_Driver * eth0 = new Ethernet_Driver_LPC23xx();

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
   threadfactory.spawnThread(ETH_MAX_FLEN+1000, 1,ethernetReceiver,&eth_incoming);

   //threadfactory.spawnThread(1000,30,threadA,&box);
   //threadfactory.spawnThread(1000,30,threadB,&box);

   eth0->install( irq_interrupt_handler );
   irqs.add( eth0 );

   printf("#Starting Scheduler\n");
   scheduler();            //This function will never return.
}

void threadA( Mailbox<Test> * box )
{
   Test t;
   t.a = 0;
   t.b = 13;
   while(1)
   {
   		box->send( 250, &t );
		t.a++;
		t.b++;
   }
}

void threadB( Mailbox<Test> * box )
{
   Test	t;
   while(1)
   {
   		box->recv( -1, &t );
   		sendchar(t.a%26 + 'a');
		sendchar(t.b%26 + 'a');
		sleep(500);
   }
}

void ethernetReceiver( Mailbox<EthernetFrame> * eth_incoming )
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
				
				printf( "--- Ethernet Frame header ---\n" );
				
				unsigned short type = frame.getEtherType();
				if( type >= 0x0600 )
				{
					//Type
					printf( "Type                    : 0x%0.4X ", type );
	
					//What is the type?
					switch(type)
					{
	
						case 0x0800:
							printf( "(IPv4)"	 );
							break;
						case 0x0806:
							printf( "(ARP)"	 );
							break;
						case 0x86DD:
							printf( "(IPv6)"	 );
							break;
						default:
							printf( "(Unknown)"	 );
					}
	
					printf( "\n" );
				}
				else
				{
					//Length
					printf( "Length                  : %d bytes\n", type );
				}
			}
			eth0->endReadFrame();
		}
	}
#if 0
	/*		 
			printf( "Destination Address     : " );
			printMAC( framebuffer + ETH_DA_OFS );
			printf( "\n" );
			
			printf( "Source Address          : " );
			printMAC( framebuffer + ETH_SA_OFS );
			printf("\n");
   */
			
 	/*
			printf( "\n--- Ethernet Data ---\n" );
			for( i = ETH_HEADER_SIZE; i < len; ++i )
			{
				if( i != ETH_HEADER_SIZE && (i - ETH_HEADER_SIZE) % 24 == 0 )
				{
					printf( "\n" );
				}
				
				printf( "%0.2x ", frame[i] );
			}
			   */
#endif
}

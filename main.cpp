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

//http://www.keil.com/support/man/docs/armlib/armlib_Chdfjddj.htm

ThreadFactory threadfactory;
struct Test
{
	uint32_t a;
	uint32_t b;	
};
Mailbox<Test> box(2);

void threadA( Mailbox<Test> * box );
void threadB( Mailbox<Test> * box );

void ethernetSender();
void ethernetReceiver( Mailbox<EthernetFrame> * eth_incoming );

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
   
   printf("#Init Network (This may take a while)\n");
   ethernet_init();

   //System threads
   threadfactory.spawnThread(ETH_MAX_FLEN+1000, 1,ethernetReceiver,&eth_incoming);

   //threadfactory.spawnThread(1000,30,threadA,&box);
   //threadfactory.spawnThread(1000,30,threadB,&box);


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
	uint8_t frame_buffer[sizeof(EthernetFrame)];
	EthernetFrame * frame( reinterpret_cast<EthernetFrame *>(frame_buffer) );

	while(true)
	{	
		if( eth_incoming->recv( -1, frame ) )
		{
   			printf( "--- Ethernet Frame header ---\n" );
	/*		 
			printf( "Destination Address     : " );
			printMAC( framebuffer + ETH_DA_OFS );
			printf( "\n" );
			
			printf( "Source Address          : " );
			printMAC( framebuffer + ETH_SA_OFS );
			printf("\n");
   *//*
			unsigned short type = frame->getEtherType();//(*(framebuffer + ETH_TYPE_OFS) << 8) + *(framebuffer + ETH_TYPE_OFS + 1);
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
			}*/
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

			frame->~EthernetFrame();
		}
		else
		{
			printf("! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
			printf("INTERNAL ERROR! Failed to recv packet in ethernetReceiver.\n");
			printf("! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
		}
	}
}

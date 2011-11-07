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
#include "SROSpp/uart1_driver.hpp"
#include "SROSpp/ethernet_handler.hpp"
#include "SROSpp/arp_handler.hpp"

ThreadFactory threadfactory;

void ethernetSender();
void ethernetReceiver();
void arpRequestThread();
void arpRecieveThread();
void userThread( UART1_Driver * console );

uint8_t const mymacaddr[6] = {0x00,0x11,0x22,0x33,0x44,0x55};
Ethernet_Driver * const eth0 = new Ethernet_Driver_LPC23xx(mymacaddr);
UART1_Driver * const uart1 = new UART1_Driver();
Ethernet_Handler eth_handler( eth0 );

uint8_t const myipaddr[4] = {192,168,0,13};
ARP_Handler arp_handler( &eth_handler, myipaddr );

int main(void)
{
	LED_Init();
	LED_Out(0x55);

   uart1->install( irq_interrupt_handler );

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
   threadfactory.spawnThread(100, 1,ethernetReceiver);
   threadfactory.spawnThread(100, 10,ethernetSender);
   threadfactory.spawnThread(400, 21,arpRequestThread);
   threadfactory.spawnThread(400, 20,arpRecieveThread);
   threadfactory.spawnThread(1000, 100,userThread,uart1);

   eth_handler.addListener( &arp_handler );

   eth0->install( irq_interrupt_handler );
   
   irqs.add( eth0 );
   irqs.add( uart1 );

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

void arpRequestThread()
{
	arp_handler.requestThread();
}

void arpRecieveThread()
{
	arp_handler.packetRecieveThread();
}

IPAddress readIP( UART1_Driver * console )
{
   //uint_fast8_t points[3] = {0};
   uint8_t octets[4] = {0};
   uint_fast8_t octet = 0;
   uint_fast8_t chars = 0;

   while( true )
   {
      char ch = 0;
      while( (ch < '0' || ch > '9') && ch != '.' && ch != '\n' && ch != '\r' )
      {
         ch = console->getC();
      }
      
      if( ch == '.' )
      {
         if( octet < 3 )
		 {
			if( chars > 0 )
			{
				printf(".");
				//points[octet] = chars;
				octet++;
				chars = 0;
			}
		 }
      }
	  else if( ch == '\n' || ch == '\r' )
	  {
	  	if( octet == 3 )
		{
			printf("\n");
			return 	IPAddress(octets);
		}
	  }
      else
      {
         if( (octets[octet] == 25 && ch <= '5') || octets[octet] < 25 )
		 {
		 	printf("%c", ch );
			octets[octet] *= 10;
			octets[octet] += (ch - '0');
			chars++;
		 }
      }
   }
}

void userThread( UART1_Driver * console )
{
	unsigned char addr[4] = {192,168,0,123};
	IPAddress iaddress( addr );
	EthernetAddress eaddress;

	while(true)
	{
      printf("\n\n");
      printf("1. Resolve IP Address\n");
      printf("2. Display ARP Cache\n");
      printf("3. Clear ARP Cache\n");
      printf("\n>> ");
      
      char ch = 0;
      while( ch < '1' || ch > '3' )
      {
         ch = console->getC();
      }
      
      printf( "%c\n\n", ch );
      
      switch( ch )
      {
         case '1':
            iaddress = readIP( console );
			printf("Resolving...\n");
            eaddress = arp_handler.request( iaddress );
			if( !(eaddress == ETHERNET_UNCONFIGURED) )
			{
      			eaddress.print();
      			printf("\n");
			}
			else
			{
				printf("HOST UNREACHABLE: Request Timeout\n");
			}
            break;
         case '2':
            arp_handler.print();
            break;
         case '3':
            arp_handler.clear();
            break;
         default:
            break;
      }
	}
}


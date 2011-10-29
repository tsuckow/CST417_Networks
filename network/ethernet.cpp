#include <SROSpp/ethernet.hpp>
#include <SROSpp/mailbox.hpp>
#include <LPC23xx.h>

#include <stdio.h>
#include <stdint.h>

Mailbox<EthernetFrame> eth_incoming(4);

static uint8_t framebuffer[ETH_MAX_FLEN];
static uint8_t frame[sizeof(EthernetFrame)];

extern "C" void irq_interrupt_handler(void);

//! \brief Initilize the ethernet system
void ethernet_init(void)
{
	Init_EMAC();

	//Interrupt
	VICVectAddr21  = (unsigned long)irq_interrupt_handler;
  	VICVectPriority21  = 15;
  	VICIntEnable  = (1  << 21);
}

//! \brief Prints the mac address to stdout
void printMAC( unsigned char * addr )
{
	printf( "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X  :  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5] );

	if( addr[0] & 0x01 )
	{
		printf(	"Multicast " );
	}
	else
	{
		printf(	"Unicast   " );
	}

	if( addr[0] & 0x02 )
	{
		printf(	"Local" );
	}
	else
	{
		printf(	"Global" );
	}
}

//!
//! \brief Ethernet Interrupt Handler
//!
//! \note There seem to be a lot of recieve errors but they don't appear to affect anything
//!
void ethernet_interrupt_handler(void)
{
	if( MAC_INTSTATUS & INT_RX_OVERRUN )
	{
		MAC_INTCLEAR = INT_RX_OVERRUN;
		printf( "ETH ERR: RX Overrun\n" );
	}
	
	if( MAC_INTSTATUS & INT_RX_ERR )
	{
		MAC_INTCLEAR = INT_RX_ERR;
		//This is normal because of packets with type field
		//printf( "ETH ERR: RX Error\n" );
	}
	
	if( MAC_INTSTATUS & INT_RX_FIN ) //RX Finished Process Descriptors
	{
		MAC_INTCLEAR = INT_RX_FIN;
		//Printing doesn't help the cause.
		//printf( "ETH: RX FIN, About to drop packet.\n" );
	}
	
	if( MAC_INTSTATUS & INT_RX_DONE )
	{
		MAC_INTCLEAR = INT_RX_DONE;

		while( CheckFrameReceived() )
		{
			unsigned short len;
			unsigned short i;
			
			len = StartReadFrame();

			if( len > ETH_MAX_FLEN )
			{
				printf( "!!!TOO BIG" );
				len = ETH_MAX_FLEN;
			}
			
			CopyFromFrame_EMAC(framebuffer,len);
			
			EndReadFrame();

			EthernetFrame * myframe = new(&frame) EthernetFrame( framebuffer, len );

			if( !eth_incoming.send( 0, myframe ) )
			{
				myframe->~EthernetFrame();
				 printf("INT: Incoming packet dropped. Buffer full.\n");	
			}
		}
	}
	
	if( MAC_INTSTATUS & INT_TX_UNDERRUN )
	{
		MAC_INTCLEAR = INT_TX_UNDERRUN;
		printf( "ETH ERR: TX Underrun\n" );
	}
	
	if( MAC_INTSTATUS & INT_TX_ERR )
	{
		MAC_INTCLEAR = INT_TX_ERR;
		printf( "ETH ERR: TX Error\n" );
	}
	
	if( MAC_INTSTATUS & INT_TX_FIN ) //TX Finished Process Descriptors
	{
		MAC_INTCLEAR = INT_TX_FIN;
		printf( "ETH: TX FIN\n" );
	}
	
	if( MAC_INTSTATUS & INT_TX_DONE )
	{
		MAC_INTCLEAR = INT_TX_DONE;
		printf( "ETH: TX DONE\n" );
	}
	
	if( MAC_INTSTATUS & INT_SOFT_INT )
	{
		MAC_INTCLEAR = INT_SOFT_INT;
		printf( "ETH: SOFT INT\n" );
	}
	
	if( MAC_INTSTATUS & INT_WAKEUP )
	{
		MAC_INTCLEAR = INT_WAKEUP;
		printf( "ETH: WAKE\n" );
	}
}


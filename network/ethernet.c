#include "ethernet.h"
#include "EMAC.h"
#include "isr.h"
#include <LPC23xx.h>

#include <stdio.h>

unsigned char frame[ETH_MAX_FLEN];

void ethernet_init(void)
{
	Init_EMAC();

	//Interrupt
	VICVectAddr21  = (unsigned long)irq_interrupt_service_routine;
  	VICVectPriority21  = 15;
  	VICIntEnable  = (1  << 21);
}

void printMAC( unsigned char * addr )
{
	printf( "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5] );
}

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
		printf( "ETH ERR: RX Error\n" );
	}
	
	if( MAC_INTSTATUS & INT_RX_FIN ) //RX Finished Process Descriptors
	{
		MAC_INTCLEAR = INT_RX_FIN;
		printf( "ETH: RX FIN\n" );
	}
	
	if( MAC_INTSTATUS & INT_RX_DONE )
	{
		MAC_INTCLEAR = INT_RX_DONE;
		
		while( CheckFrameReceived() )
		{
			unsigned short len;
			unsigned short i;
			printf( "\n--- Ethernet Frame header ---\n" );
			
			len = StartReadFrame();
			
			if( len > ETH_MAX_FLEN )
				len = ETH_MAX_FLEN;
			
			CopyFromFrame_EMAC(frame,len);
			
			EndReadFrame();
			
			printf( "Destination Address     : " );
			printMAC( frame + ETH_DA_OFS );
			printf( "\n" );
			
			printf( "Source Address          : " );
			printMAC( frame + ETH_SA_OFS );
			printf("\n");

			if( *(frame + ETH_TYPE_OFS) >= 6 )
			{
				//Type
				printf( "Type                    : 0x%0.2X%0.2X\n", *(frame + ETH_TYPE_OFS), *(frame + ETH_TYPE_OFS + 1) );
			}
			else
			{
				//Length
				unsigned short len = *(frame + ETH_TYPE_OFS) << 16 + *(frame + ETH_TYPE_OFS + 1);
				
				printf( "Length                  : %d bytes\n", len );
			}
			
			printf( "\n--- Ethernet Data ---\n" );
			for( i = ETH_HEADER_SIZE; i < len; ++i )
			{
				if( i != ETH_HEADER_SIZE && (i - ETH_HEADER_SIZE) % 24 == 0 )
				{
					printf( "\n" );
				}
				
				printf( "%0.2x ", frame[i] );
			}
			printf( "\n" );
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


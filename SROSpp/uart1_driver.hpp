#pragma once

#include "driver.hpp"

class UART1_Driver : public Driver
{
protected:
   Mailbox<char> box;
   
public:
   UART1_Driver() : box( 8 )
   {
   }

	virtual ~UART1_Driver(){}
	
	virtual void install( void (*handler)(void) )
	{
      PINSEL0 |= 0x40000000;               /* Enable TxD1                       */
      PINSEL1 |= 0x00000001;               /* Enable RxD1                       */
    
      U1FDR    = 0;                          /* Fractional divider not used       */
      U1LCR    = 0x83;                       /* 8 bits, no Parity, 1 Stop bit     */
    //U1DLL    = 78;                         /* 9600 Baud Rate @ 12.0 MHZ PCLK    */
    //U1DLL    = 39;                         /* 19200 Baud Rate @ 12.0 MHZ PCLK   */
      U1DLL    = 13;                         /* 57600 Baud Rate @ 12.0 MHZ PCLK   */
      U1DLM    = 0;                          /* High divisor latch = 0            */
      U1LCR    = 0x03;                       /* DLAB = 0                          */
  
		U1IER = 0x01;                       /* Enable RDA */
		VICVectAddr7 = (unsigned int)handler;
		VICVectPriority7 = 15;
		VICIntEnable  = (1  << 7);          /* Enable Interrupt                    */
	}
	
	virtual void irq_handler()
	{
      uint32_t reg;
      while( ( (reg = U1IIR) & 0x01) == 0 )
      {
         if( (reg & 0xE) != 0x4 )
         {
            printf( "FAULT! Unknown UART INT %.8x\n", reg );
         }
         
         while (U1LSR & 0x01)
         {
            char tmp = U1RBR;
            box.send( 0, &tmp );
         }
      }
	}	
   
   char getC( bool * valid = 0, int timeout = -1 )
   {
      char tmp;
      bool ok = box.recv( timeout, &tmp );
      if( valid != 0 ) *valid = ok;
      return tmp;
   }
};

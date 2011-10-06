#include <LPC23xx.H>                    /* LPC23xx definitions                */

#include <stdio.h>
#include <stdlib.h>

#include <led/led.h>
#include <serial/serial.h>
#include <lcd/lcd.h>
#include "timer.h"
#include "network/ethernet.h"

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

   srand(1);

   printf("#Init Timer\n");
   timer_init();
   
   printf("#Init Network\n");
   ethernet_init();

   while(1)
   {
   }
}

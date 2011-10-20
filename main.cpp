#include <LPC23xx.H>                    /* LPC23xx definitions                */

#include <stdio.h>
#include <stdlib.h>

#include <rtos.h>
#include <rwLock.h>

#include <led/led.h>
#include <serial/serial.h>
#include <lcd/lcd.h>
#include "timer.h"
#include "network/ethernet.h"

#include "SROSpp/threadfactory.h"
#include "SROSpp/mailbox.h"

ThreadFactory threadfactory;
Mailbox<uint32_t> box(2);

void functionLowThread();

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

   threadfactory.spawnThread(1000,30,functionLowThread);

   scheduler();            //This function will never return.
}

void functionLowThread()
{
   printf("Low Priority\n");
   while(1);
}


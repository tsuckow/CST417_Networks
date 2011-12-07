#include <stdio.h>

void Undef_Handler()
{
   printf("Undef_Handler!");
   while(1);
}

void SWI_Handler()
{
   printf("DERP!");
   while(1);
}

void PAbt_Handler()
{
   printf("SWI_Handler!");
   while(1);
}

void DAbt_Handler()
{
   printf("DAbt_Handler!");
   while(1);
}

void IRQ_Handler()
{
   printf("IRQ_Handler!");
   while(1);
}

void FIQ_Handler()
{
   printf("FIQ_Handler!");
   while(1);
}

#pragma once

class Driver
{
	public:
		virtual ~Driver(){}
		virtual void install( void (*handler)(void) ) = 0;
		virtual void irq_handler() = 0;
};

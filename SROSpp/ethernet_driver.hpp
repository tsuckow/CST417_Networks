#pragma once

#include "driver.hpp"
#include "semaphore.hpp"

class Ethernet_Driver : public Driver
{
	protected:
		Semaphore sem_recv;
		Semaphore sem_tx;
	public:
		Ethernet_Driver() : sem_recv( 0 )
		{
		}
		
		virtual ~Ethernet_Driver(){}
		virtual bool init() = 0;
		
		bool recv_wait( int32_t timeout )
		{
			return sem_recv.wait( timeout );	
		}
		
		virtual bool isFrameAvailable() = 0;
		virtual unsigned short beginReadFrame(void) = 0;
		virtual uint8_t * readFramePointer() = 0;
		virtual void endReadFrame(void) = 0;
		virtual void sendFrame( uint8_t const * frame, uint_fast16_t size ) = 0;
};

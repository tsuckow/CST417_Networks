#pragma once

#include <list>
#include <rtos.h>
#include <stdint.h>
#include <stdlib.h>

class Semaphore
{
private:
	semaphoreObject_t box;
public:
	Semaphore( uint32_t init_size = 0 )
	{
		semaphoreObjectInit( &box, init_size );
	}
		
	void signal()
	{
		semaphoreObjectPost( &box );
	}

	/**
	\param timeout Number of timer ticks before timeout or inf if negative

	\return success
	*/
	bool wait( int32_t timeout = -1 )
	{
		return semaphoreObjectPend( &box, timeout );
	}
};

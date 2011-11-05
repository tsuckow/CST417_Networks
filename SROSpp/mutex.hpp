#pragma once

#include <rtos.h>
#include <stdint.h>
#include <stdlib.h>

class Mutex
{
private:
	mutexObject_t box;
public:
    //! \param init_flag 1 == Availible, 0 == Locked
	Mutex( uint32_t init_flag = 1 )
	{
		mutexObjectInit( &box, init_flag );
	}

	/**
	\param timeout Number of timer ticks before timeout or inf if negative

	\return success
	*/	
	bool lock( int32_t timeout = -1 )
	{
		return mutexObjectLock( &box, timeout );
	}

	void release()
	{
		return mutexObjectRelease( &box );
	}
};

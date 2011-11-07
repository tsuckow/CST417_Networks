#pragma once

#include <rtos.h>
#include <stdint.h>
#include <stdlib.h>

template<typename T>
class Mailbox
{
private:
	mailboxObject_t box;	
	T * buffer;
public:
	Mailbox( unsigned int size )
	{
		buffer = reinterpret_cast<T*>( malloc( sizeof(T) * size ) );
		mailboxObjectInit( &box,  
                       (int8*)buffer, 
                       sizeof(T) * size, 
                       sizeof(T) );
	}

	//Send a semaphore in T to unblock thread when buffer is safe to delete.

	/**
	\param timeout Number of timer ticks before timeout or inf if negative

	\return success
	*/
	bool send( int32_t timeout, T * message )
	{
		return mailboxObjectPost( &box,	timeout, message );
	}

	bool recv( int32_t timeout, T * message )
	{
		return mailboxObjectPend( &box, timeout, message);
	}
};

#pragma once

#include <list>
#include <rtos.h>
#include <stdint.h>


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
	bool send()

	bool recv()
};

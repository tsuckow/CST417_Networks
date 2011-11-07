//Builds threads

#pragma once

#include "ll.hpp"
#include <rtos.h>
#include <stdint.h>


class ThreadFactory
{
private:
	struct Thread
	{
		threadObject_t t;
		int32_t * stack;
	};
	//ll<Thread *> threads;	

public:
	ThreadFactory()
	{
	}

	template<typename P1, typename P2, typename P3, typename P4>
	void spawnThread( unsigned int stacksize, uint32_t priority, void (*func)(P1,P2,P3,P4), P1 p1, P2 p2, P3 p3, P4 p4 )
	{
		Thread * tobj = new Thread();
		tobj->stack = new int32_t[stacksize];
		//threads.push_front(tobj);

		threadObjectCreate(&(tobj->t),
                     (void *)func,
                     *reinterpret_cast<int32_t *>(&p1),//Param 1
                     static_cast<int32_t>(p2),//Param 2
                     static_cast<int32_t>(p3),//Param 3
                     static_cast<int32_t>(p4),//Param 4
                     &tobj->stack[stacksize],
                     priority,
                     INITIAL_CPSR_ARM_FUNCTION,
                     "Lthread");
	}
	
	template<typename P1, typename P2, typename P3>
	void spawnThread( unsigned int stacksize, uint32_t priority, void (*func)(P1,P2,P3), P1 p1, P2 p2, P3 p3 )
	{
		spawnThread( stacksize, priority, reinterpret_cast<void (*)(P1,P2,P3,int)>(func), p1, p2, p3, 0 );
	}
	
	template<typename P1, typename P2>
	void spawnThread( unsigned int stacksize, uint32_t priority, void (*func)(P1,P2), P1 p1, P2 p2 )
	{
		spawnThread( stacksize, priority, reinterpret_cast<void (*)(P1,P2,int,int)>(func), p1, p2, 0, 0 );
	}
	
	template<typename P1>
	void spawnThread( unsigned int stacksize, uint32_t priority, void (*func)(P1), P1 p1 )
	{
		spawnThread( stacksize, priority, reinterpret_cast<void (*)(P1,int,int,int)>(func), p1, 0, 0, 0 );
	}

	void spawnThread( unsigned int stacksize, uint32_t priority, void (*func)(void) )
	{
		spawnThread( stacksize, priority, reinterpret_cast<void (*)(int,int,int,int)>(func), 0, 0, 0, 0 );
	}
};

#pragma once

#include <list>
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
	std::list<Thread *> threads;	

public:
	ThreadFactory()
	{
	}

	void spawnThread( unsigned int stacksize, uint32_t priority, void (*func)(void) )
	{
		Thread * tobj = new Thread();
		tobj->stack = new int32_t[stacksize];
		threads.push_back(tobj);

		threadObjectCreate(&(tobj->t),
                     (void *)func,
                     0,//Param 1
                     0,//Param 2
                     0,//Param 3
                     0,//Param 4
                     tobj->stack,
                     priority,
                     INITIAL_CPSR_ARM_FUNCTION,
                     "Lthread");
	}

	template<typename P1>
	void spawnThread( unsigned int stacksize, uint32_t priority, void (*func)(P1), P1 p1 )
	{
		Thread * tobj = new Thread();
		tobj->stack = new int32_t[stacksize];
		threads.push_back(tobj);

		threadObjectCreate(&(tobj->t),
                     (void *)func,
                     (int32_t)p1,//Param 1
                     0,//Param 2
                     0,//Param 3
                     0,//Param 4
                     &tobj->stack[stacksize],
                     priority,
                     INITIAL_CPSR_ARM_FUNCTION,
                     "Lthread");
	}
};

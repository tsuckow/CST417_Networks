//Make new and delete thread safe

#include <rtos.h>

#define NUM_MUTEXES	5
mutexObject_t mutexes[NUM_MUTEXES];

int _mutex_init_calls=0;
int _mutex_aqui_calls=0;
int _mutex_rele_calls=0;


#define mutex	void
//typedef volatile int32 mutex;

int _mutex_initialize(mutex* m) {
	static unsigned int allocated=0;
	++_mutex_init_calls;
	if (allocated < NUM_MUTEXES) {
		mutexObjectInit(&mutexes[allocated],1);
		*(void**)m = (void*)(&mutexes[allocated]);
		++allocated;
		return 1;
	}
	return 0;
}

void _mutex_acquire(mutex* m)
{
	++_mutex_aqui_calls;
	if( isSROSRunning() )
		mutexObjectLock((mutexObject_t*)(*(void**)m),-1);
}

void _mutex_release(mutex* m) {
	++_mutex_rele_calls;
	if( isSROSRunning() )
		mutexObjectRelease((mutexObject_t*)(*(void**)m));
}

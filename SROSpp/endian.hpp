
#pragma once

inline uint8_t load8( uint8_t * ptr )
{
	return ptr[0];
}

inline uint16_t loadBig16( uint8_t * ptr )
{
	return (ptr[0] << 8) + ptr[1];
}

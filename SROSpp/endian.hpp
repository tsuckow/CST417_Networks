
#pragma once

inline uint8_t load8( uint8_t * ptr )
{
   return ptr[0];
}

inline uint16_t loadBig16( uint8_t * ptr )
{
   return (ptr[0] << 8) + ptr[1];
}

inline void store8( uint8_t * ptr, uint8_t value )
{
   ptr[0] = value;
}

inline void storeBig16( uint8_t * ptr, uint16_t value )
{
   ptr[1] = static_cast<uint8_t>( value );
   ptr[0] = static_cast<uint8_t>( value >> 8 );
}



#pragma once

inline uint8_t load8( uint8_t const * ptr )
{
   return ptr[0];
}

inline uint16_t loadBig16( uint8_t const * ptr )
{
   return (ptr[0] << 8) + ptr[1];
}

inline uint32_t loadBig32( uint8_t const * ptr )
{
   return (loadBig16(ptr) << 16) + loadBig16(ptr+2);
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

inline void storeBig32( uint8_t * ptr, uint32_t value )
{
   ptr[3] = static_cast<uint8_t>( value );
   ptr[2] = static_cast<uint8_t>( value >> 8  );
   ptr[1] = static_cast<uint8_t>( value >> 16 );
   ptr[0] = static_cast<uint8_t>( value >> 24 );
}
